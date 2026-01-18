#!/usr/bin/env python3
"""
Performance Benchmarking Script
Compares CPU vs FPGA-accelerated CNN inference
"""

import subprocess
import json
import statistics
import time
from pathlib import Path

class PerformanceBenchmark:
    def __init__(self, cpu_binary, fpga_binary, iterations=100):
        self.cpu_binary = cpu_binary
        self.fpga_binary = fpga_binary
        self.iterations = iterations
        self.results = {
            'cpu': {'latencies': [], 'throughput': 0},
            'fpga': {'latencies': [], 'throughput': 0}
        }
    
    def run_benchmark(self, binary_path, name):
        """Run benchmark for a given binary"""
        print(f"\n{'='*60}")
        print(f"Running {name} benchmark ({self.iterations} iterations)...")
        print(f"{'='*60}")
        
        latencies = []
        
        for i in range(self.iterations):
            start = time.time()
            
            try:
                result = subprocess.run(
                    [binary_path],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
                
                end = time.time()
                latency_ms = (end - start) * 1000
                latencies.append(latency_ms)
                
                if (i + 1) % 10 == 0:
                    print(f"  Progress: {i+1}/{self.iterations} iterations")
                
            except subprocess.TimeoutExpired:
                print(f"  Warning: Iteration {i+1} timed out")
                continue
            except Exception as e:
                print(f"  Error in iteration {i+1}: {e}")
                continue
        
        return latencies
    
    def analyze_results(self):
        """Analyze and display benchmark results"""
        print(f"\n{'='*60}")
        print("PERFORMANCE ANALYSIS")
        print(f"{'='*60}\n")
        
        # CPU results
        cpu_latencies = self.results['cpu']['latencies']
        if cpu_latencies:
            cpu_avg = statistics.mean(cpu_latencies)
            cpu_std = statistics.stdev(cpu_latencies) if len(cpu_latencies) > 1 else 0
            cpu_p95 = sorted(cpu_latencies)[int(len(cpu_latencies) * 0.95)]
            cpu_fps = 1000.0 / cpu_avg
            
            print("CPU Baseline:")
            print(f"  Average Latency:    {cpu_avg:.2f} ms")
            print(f"  Std Deviation:      {cpu_std:.2f} ms")
            print(f"  95th Percentile:    {cpu_p95:.2f} ms")
            print(f"  Throughput:         {cpu_fps:.2f} FPS")
        
        # FPGA results
        fpga_latencies = self.results['fpga']['latencies']
        if fpga_latencies:
            fpga_avg = statistics.mean(fpga_latencies)
            fpga_std = statistics.stdev(fpga_latencies) if len(fpga_latencies) > 1 else 0
            fpga_p95 = sorted(fpga_latencies)[int(len(fpga_latencies) * 0.95)]
            fpga_fps = 1000.0 / fpga_avg
            
            print("\nFPGA Accelerated:")
            print(f"  Average Latency:    {fpga_avg:.2f} ms")
            print(f"  Std Deviation:      {fpga_std:.2f} ms")
            print(f"  95th Percentile:    {fpga_p95:.2f} ms")
            print(f"  Throughput:         {fpga_fps:.2f} FPS")
        
        # Speedup calculation
        if cpu_latencies and fpga_latencies:
            speedup = cpu_avg / fpga_avg
            print(f"\n{'='*60}")
            print(f"SPEEDUP: {speedup:.2f}x")
            print(f"{'='*60}")
            
            if speedup >= 2.0:
                print("✓ Target speedup (2x) ACHIEVED!")
            else:
                print(f"⚠ Target speedup not met (current: {speedup:.2f}x, target: 2.0x)")
        
        return {
            'cpu': {
                'avg_latency_ms': cpu_avg if cpu_latencies else 0,
                'std_dev_ms': cpu_std if cpu_latencies else 0,
                'p95_latency_ms': cpu_p95 if cpu_latencies else 0,
                'fps': cpu_fps if cpu_latencies else 0
            },
            'fpga': {
                'avg_latency_ms': fpga_avg if fpga_latencies else 0,
                'std_dev_ms': fpga_std if fpga_latencies else 0,
                'p95_latency_ms': fpga_p95 if fpga_latencies else 0,
                'fps': fpga_fps if fpga_latencies else 0
            },
            'speedup': speedup if (cpu_latencies and fpga_latencies) else 0
        }
    
    def run(self):
        """Run complete benchmark suite"""
        # Run CPU benchmark
        self.results['cpu']['latencies'] = self.run_benchmark(
            self.cpu_binary, "CPU Baseline"
        )
        
        # Run FPGA benchmark
        self.results['fpga']['latencies'] = self.run_benchmark(
            self.fpga_binary, "FPGA Accelerated"
        )
        
        # Analyze and display results
        summary = self.analyze_results()
        
        # Save results to JSON
        output_file = "benchmark_results.json"
        with open(output_file, 'w') as f:
            json.dump(summary, f, indent=2)
        
        print(f"\nResults saved to {output_file}")
        
        return summary

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='CNN Performance Benchmark')
    parser.add_argument('--cpu-binary', type=str, 
                       default='../software/bin/cnn_inference_cpu',
                       help='Path to CPU baseline binary')
    parser.add_argument('--fpga-binary', type=str,
                       default='../software/bin/cnn_inference_hw',
                       help='Path to FPGA accelerated binary')
    parser.add_argument('--iterations', type=int, default=100,
                       help='Number of benchmark iterations')
    
    args = parser.parse_args()
    
    # Verify binaries exist
    if not Path(args.cpu_binary).exists():
        print(f"Error: CPU binary not found: {args.cpu_binary}")
        return 1
    
    if not Path(args.fpga_binary).exists():
        print(f"Error: FPGA binary not found: {args.fpga_binary}")
        return 1
    
    # Run benchmark
    benchmark = PerformanceBenchmark(
        args.cpu_binary,
        args.fpga_binary,
        args.iterations
    )
    
    benchmark.run()
    
    return 0

if __name__ == '__main__':
    exit(main())
