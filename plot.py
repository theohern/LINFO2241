import numpy as np
import matplotlib.pyplot as plt
import scipy.linalg

Flag = "phase3"

if (Flag == "mult"):
    x = [1000, 1100, 1200, 1300, 1400, 1500, 1600, 1700,1800, 1900, 2000]
    y = [6.5, 10.5, 17.24, 17.55, 24.32, 35.76, 52.43, 73.05, 95.21,103.14 , 129.59]
    y2 = [3.54, 4.46, 6.48, 6.76, 9.92, 12.63, 13.65, 23.51, 32.07, 33.71, 47.65]
    plt.figure()
    plt.title("time compared to the type of multiplication for C = A x B")
    plt.xlabel("size of matrix")
    plt.ylabel("time (s)")

    plt.plot(x, y, label="line * column")
    plt.plot(x, y2, label="line * line")
    plt.gca().legend(('line * column','line * line'))
    plt.show()

if (Flag == "result"):
    x = [50,100,150,200,250,300,350]
    y = [4.07, 8.24, 10.05, 16.26, 20.32, 24.72, 37.59]
    y2 = [2.27, 4.25, 7.04, 7.51, 10.16, 13.35, 19.06]
    plt.figure()
    plt.title("comparison between Phase1 and Phase2")
    plt.xlabel("rate (1/s)")
    plt.ylabel("time (s)")
    plt.plot(x, y)
    plt.plot(x, y2)
    plt.gca().legend(('phase 1','phase 2'))
    plt.show()


if (Flag == "file"):
    x = [16, 256, 4096, 128*128,65536,512*512,850*850, 950*950, 1024*1024]
    y = [0.02, 0.02,0.02,0.03,0.54, 0.79, 8.95,11.55, 13.14]
    plt.figure()
    plt.title("time compared to the size of the file")
    plt.xlabel("size of the file")
    plt.ylabel("time (s)")
    plt.plot(x, y)
    plt.show()


if (Flag == "nthreads"):
    x = [1, 2, 3, 4]
    y = [44.12, 45.68, 42.64, 46.47]
    plt.figure()
    plt.title("time compared to number of threads")
    plt.xlabel("number of threads")
    plt.ylabel("time (s)")
    plt.plot(x, y)
    plt.show() 


if (Flag == "phase3"):
    file = open("time.txt", 'r')
    ybis = file.readlines()
    x = [16,32,64,128,256,512,1024]
    plt.title("Request latency for rate 4 and time 5")
    plt.xlabel("size of the files")
    plt.ylabel("time (ms)")
    y = np.zeros(len(ybis))
    for i in range(len(ybis)):
        y[i] = int(ybis[i][:-1])
        y[i] = y[i]/1000
    plt.plot(x, y[0:7], label="server without SIMD")
    plt.plot(x, y[7:14], label="server with SIMD")
    plt.legend()
    #plt.gca().legend(('line * column','line * line'))
    #plt.show()
    plt.savefig("graph.png")

