import matplotlib.pyplot as plt
import argparse
import os
import numpy as np
import tkinter
import sys

if getattr(sys, 'frozen', False):
    import matplotlib
    matplotlib.use('TkAgg')

def main():
    parser = argparse.ArgumentParser(description='2d plot from file \"[x;y\nx;y...]')
    parser.add_argument('filename', type=str, help='Input file (format: x;y)')
    parser.add_argument('--delta', type=float, default=1.0, help='Sampling interval in milliseconds (default: 1.0 ms)')
    parser.add_argument('--save', action='store_true', help='Save plot without showing')
    args = parser.parse_args()

    x, y = [], []

    with open(args.filename, 'r') as f:
        for line in f:
            if ';' in line:
                parts = line.strip().split(';')
                if len(parts) == 2:
                    try:
                        x.append(int(parts[0]))
                        y.append(int(parts[1]))
                    except ValueError:
                        continue

    if not x:
        print("Invalid file format")
        
        return
    
    time_ms = np.arange(len(x)) * args.delta

    plt.figure(figsize=(12, 10))
    
    #plt.subplot(2, 1, 1)
    plt.plot(x, y, 'b-', alpha=0.7, label='Path')
    plt.plot(x, y, 'ro', markersize=2, alpha=0.3, label='Points')
    plt.xlim(0, 1920)
    plt.ylim(1080, 0)
    plt.title(f'Cursor Movement Path (Sampling: {args.delta} ms)')
    plt.xlabel('X Position')
    plt.ylabel('Y Position')
    plt.grid(True)
    plt.legend()
    
    #plt.subplot(2, 2, 3)
    #plt.plot(time_ms, x, 'g-', linewidth=1.5)
    #plt.title('X Position Over Time')
    #plt.xlabel('Time (ms)')
    #plt.ylabel('X Value')
    #plt.grid(True)
    
    #plt.subplot(2, 2, 4)
    #plt.plot(time_ms, y, 'm-', linewidth=1.5)
    #plt.title('Y Position Over Time')
    #plt.xlabel('Time (ms)')
    #plt.ylabel('Y Value')
    #plt.grid(True)
    
    plt.tight_layout()

    if args.save:
        output_file = os.path.splitext(args.filename)[0] + '_plot.png'
        plt.savefig(output_file, dpi=500)
        print(f"Plot saved to: {output_file}")
    else:
        plt.show()

if __name__ == "__main__":
    main()