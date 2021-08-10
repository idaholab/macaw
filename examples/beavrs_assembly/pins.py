# Simple script to generate a list of points
n_rows = 17
pitch = 1.25984

for i in range(n_rows):
    for j in range(n_rows):
        print(i * pitch, -j * pitch, 0)
