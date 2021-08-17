#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import subprocess
import time
import argparse
import pandas
import matplotlib.pyplot as plt
import multiprocessing
import statistics
import collections
import mooseutils

# Imported from Andrew's PR #18005 to MOOSE
# Run run_openmc_csg first to get a set of xml files for OpenMC

# Weak  : growing problem size
# Strong: constant problem size, run time should go down, measure speedup

def get_args():
    parser = argparse.ArgumentParser(description='Utility for producing results, plots, and tables for scalability study')
    parser.add_argument('--run', action='store_true', help="Perform simulations.")
    parser.add_argument('--replicates', default=10, type=int, help="Number of replicates to perform.")
    parser.add_argument('--base', default=128, type=int, help="The base number of samples to perform.")
    parser.add_argument('--memory-levels', default=6, type=int, help="Number of levels to perform for memory/timing runs, n in [base*2^0, ..., base*2^n-1].")
    parser.add_argument('--memory-cores', default=28, type=int, help="Number of processors to use for memory/timing runs.")
    parser.add_argument('--memory-threads', default=28, type=int, help="Number of processors to use for memory/timing runs.")
    parser.add_argument('--weak-levels', default=7, type=int, help="Number of processor levels to perform for weak scaling, n in [2^0,...,2^n-1].")
    parser.add_argument('--write', action='store_true', help="Toggle writing to results directory")
    return parser.parse_args()

def execute(infile, outfile, mode, samples, mpi=None, write=True, scaling='weak', parallel_mode='mpi'):
    data = collections.defaultdict(list)
    if mpi is None: mpi = [1]*len(samples)
    exe = mooseutils.find_moose_executable_recursive()
    for n_cores, n_samples in zip(mpi, samples):

        # Select parallelism type
        if parallel_mode == 'openmp':
            n_threads = n_cores
            n_mpi = 1
        elif parallel_mode == 'mpi':
            n_threads = 1
            n_mpi = n_cores
        else:
            raise ValueError('Unknown parallel mode', parallel_mode)

        # Build command
        cmd = ['mpiexec', '-n', str(n_mpi), exe, '-i', infile, '--n-threads='+str(n_threads),
               'Outputs/file_base={}'.format(mode)]

        if scaling == 'weak':
            scale = int(5 * n_cores**(0.333333)) / 5
            cmd.append('Mesh/gmg/nx={}'.format(5 * scale))
            cmd.append('Mesh/gmg/ny={}'.format(5 * scale))
            cmd.append('Mesh/gmg/nz={}'.format(5 * scale))

            # Making problem size bigger does not make problem more difficult,
            # it keeps the difficulty constant actually
            # cmd.append('Mesh/gmg/xmin={}'.format(-5 * scale))
            # cmd.append('Mesh/gmg/ymin={}'.format(-5 * scale))
            # cmd.append('Mesh/gmg/zmin={}'.format(-5 * scale))
            # cmd.append('Mesh/gmg/xmax={}'.format(5 * scale))
            # cmd.append('Mesh/gmg/ymax={}'.format(5 * scale))
            # cmd.append('Mesh/gmg/zmax={}'.format(5 * scale))

        print(' '.join(cmd))
        out = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        local = pandas.read_csv('{}.csv'.format(mode))
        data['n_cores'].append(n_cores)
        data['n_samples'].append(n_samples)
        data['mem_total'].append(local['total_mem'].iloc[1])
        data['mem_per_proc'].append(local['per_proc'].iloc[1])
        data['mem_max_proc'].append(local['max_proc'].iloc[1])
        data['run_time'].append(statistics.mean(local['run_time'].iloc[1:]))
        data['run_time_min'].append(min(local['run_time'].iloc[1:]))
        data['run_time_max'].append(max(local['run_time'].iloc[1:]))
        data['run_time*size'].append(n_cores * statistics.mean(local['run_time'].iloc[1:]))

        df = pandas.DataFrame(data, columns=data.keys())
        if write:
            df.to_csv('results/{}_{}.csv'.format(outfile, mode))


def plot(prefix, suffix, xname, yname, xlabel=None, ylabel=None, yerr=None, results=True):

    fig = plt.figure(figsize=[4,4], dpi=300, tight_layout=True)
    ax = fig.subplots()

    for i, mode in enumerate(['normal']):#, 'batch-restore', 'batch-reset']):
        data = pandas.read_csv('results/{}_{}.csv'.format(prefix, mode))
        kwargs = dict()
        kwargs['label'] = mode
        kwargs['linewidth'] = 0.5
        kwargs['color'] = 'k'
        kwargs['markersize'] = 2
        kwargs['marker'] = 'osd'[i]
        if yerr is not None:
            kwargs['elinewidth'] = 0.5
            kwargs['capsize'] = 2
            kwargs['yerr'] = [ (data[yerr[0]] - data[yname]).tolist(),
                               (data[yname] - data[yerr[1]]).tolist()]

        # Adapt for efficiency
        if (suffix == 'efficiency' or suffix == 'speedup'):
            data[yname] = data[yname][0] / data[yname]
        ax.errorbar(data[xname], data[yname], **kwargs)

    if xlabel is not None:
        ax.set_xlabel(xlabel, fontsize=10)
    if ylabel is not None:
        ax.set_ylabel(ylabel, fontsize=10)
    ax.grid(True, color=[0.7]*3)
    ax.grid(True, which='minor', color=[0.8]*3)
    ax.legend()

    outfile = '{}_{}.pdf'.format(prefix, suffix)
    fig.savefig(outfile)

def table(prefix):

    out = list()
    out.append(r'\begin{tabular}{ccccc}')
    out.append(r'\toprule')
    out.append(r'& & {} \\'.format('\multicolumn{3}{c}{time (sec.)}'))
    out.append(r'\cmidrule{3-5}')
    out.append(r'{} & {} & {} & {} & {} \\'.format('Processors', 'Simulations', 'normal', 'batch-reset', 'batch-restore'))
    out.append(r'\midrule')

    times = collections.defaultdict(list)
    for i, mode in enumerate(['normal', 'batch-reset', 'batch-restore']):
        data = pandas.read_csv('results/{}_{}.csv'.format(prefix, mode))
        for idx, row in data.iterrows():
            key = (int(row['n_samples']), int(row['n_ranks']))
            times[key].append((row['run_time'], row['run_time_min'], row['run_time_max']))

    for key, value in times.items():
        n_samples = key[0]
        n_ranks = key[1]
        normal = '{:.1f} ({:.1f}, {:.1f})'.format(*value[0])
        reset = '{:.1f} ({:.1f}, {:.1f})'.format(*value[1])
        restore = '{:.1f} ({:.1f}, {:.1f})'.format(*value[2])
        out.append(r'{} & {} & {} & {} & {} \\'.format(n_ranks, n_samples, normal, reset, restore))

    out.append(r'\bottomrule')
    out.append(r'\end{tabular}')

    with open('weak.tex', 'w') as fid:
        fid.write('\n'.join(out))

if __name__ == '__main__':

    input_file = 'infinite_medium.i'
    args = get_args()

    if (not args.run or not args.write):
        print("\nAre you missing the run and write flags on purpose?\n")
    else:
        # Save old plots
        os.system("mv *pdf results/")

    # Memory Parallel
    # if args.run:
    #     prefix = 'full_solve_memory_parallel'
    #     samples = [args.base*2**n for n in range(args.memory_levels)]
    #     mpi = [args.memory_cores]*len(samples)
    #     execute(input_file, prefix, 'normal', samples, mpi, args.replicates, args.write)

    # Weak scale
    if args.run:
        prefix = 'full_solve_weak_scale'
        mpi = [2**n for n in range(1, args.weak_levels)]
        samples = [args.base*m for m in mpi]
        execute(input_file, prefix, 'normal', samples, mpi, args.write)

    # Strong scale
    if args.run:
        prefix = 'full_solve_strong_scale'
        mpi = [2**n for n in range(1, args.weak_levels)]
        samples = [args.base*m for m in mpi]
        execute(input_file, prefix, 'normal', samples, mpi, args.write, 'strong')

    # Parallel time and memory plots
    # if False:
    #     plot('full_solve_memory_parallel', 'time',
    #          xname='n_samples', xlabel='Number of Simulations',
    #          yname='run_time', ylabel='Time (sec.)', yerr=('run_time_min', 'run_time_max'))
    #
    #     plot('full_solve_memory_parallel', 'memory',
    #          xname='n_samples', xlabel='Number of Simulations',
    #          yname='mem_per_proc', ylabel='Memory (MiB)')

    # Weak scaling plots
    if True:
        plot('full_solve_weak_scale', 'time',
             xname='n_cores', xlabel='Number of cores (-)',
             yname='run_time', ylabel='Time (sec.)', yerr=('run_time_min', 'run_time_max'))

        plot('full_solve_weak_scale', 'efficiency',
             xname='n_cores', xlabel='Number of cores (-)',
             yname='run_time', ylabel='Efficiency (-)')


        # plot('full_solve_weak_scale', 'memory',
        #      xname='n_cores', xlabel='Number of cores (-)',
        #      yname='mem_per_proc', ylabel='Memory (MiB)')

    # Strong scaling plots
    if True:
        plot('full_solve_strong_scale', 'time',
             xname='n_cores', xlabel='Number of cores (-)',
             yname='run_time', ylabel='Time (sec.)', yerr=('run_time_min', 'run_time_max'))

        plot('full_solve_strong_scale', 'speedup',
             xname='n_cores', xlabel='Number of cores (-)',
             yname='run_time', ylabel='spedup (-)')

        plot('full_solve_strong_scale', 'efficiency',
             xname='n_cores', xlabel='Number of cores (-)',
             yname='run_time*size', ylabel='Efficiency (-)')

        # plot('full_solve_strong_scale', 'memory',
        #      xname='n_cores', xlabel='Number of cores (-)',
        #      yname='mem_per_proc', ylabel='Memory (MiB)')
