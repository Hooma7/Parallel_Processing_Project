import numpy as np

num_procs = [2, 4, 8, 16, 32, 64]

eager_alpha = 7.248020490303695e-07
eager_R_p = 3192714645.8827844
eager_R_N = 2961212252.4174347
eager_beta_intra = 1/eager_R_p

rend_alpha = 8.712999999999998e-06
rend_R_p = 45543758967.0014
rend_R_N = 20640490913.89365
rend_beta_intra = 1/rend_R_p



print("=============================================================================================\nIntra-Node Communication")
for i in range(6):
    procs = num_procs[i]
    print(f"\nPerformance for Process Count: {procs}")
    for j in range(11):
        n = 2**j * 8
        t_recursive_doubling = np.log2(procs) * (eager_alpha +  n * eager_beta_intra)
        t_binom_tree = 2 * np.log2(procs) * eager_alpha + 2 * np.log2(procs) * n * eager_beta_intra
        t_rabenseifner = 2 * np.log2(procs) * eager_alpha + 2 * ((procs-1)/procs) * n * eager_beta_intra
        
        print(f"For message size: 2^{j} \tRecursive Doubling: {t_recursive_doubling}\t\tBinomial Tree Algorithm: {t_binom_tree}\t\tRabenseifner's Algorithm: {t_rabenseifner}")
    
    for j in range(10):
        n = 2**(j+11) * 8
        t_recursive_doubling = np.log2(procs) * (rend_alpha +  n * rend_beta_intra)
        t_binom_tree = 2 * np.log2(procs) * rend_alpha + 2 * np.log2(procs) * n * rend_beta_intra
        t_rabenseifner = 2 * np.log2(procs) * rend_alpha + 2 * ((procs-1)/procs) * n * rend_beta_intra
        
        print(f"For message size: 2^{j+11} \tRecursive Doubling: {t_recursive_doubling}\t\tBinomial Tree Algorithm: {t_binom_tree}\t\tRabenseifner's Algorithm: {t_rabenseifner}")
        
        
        
print("=============================================================================================\nInter-Node Communication")
for i in range(6):
    procs = num_procs[i]
    ppn = procs/2
    print(f"\nPerformance for Process Count: {procs}")
    for j in range(11):
        n = 2**j * 8
        t_recursive_doubling = np.log2(procs) * (eager_alpha +  n * (ppn/eager_R_N))
        t_binom_tree = 2 * np.log2(procs) * eager_alpha + 2 * np.log2(procs) * n * (ppn/eager_R_N)
        t_rabenseifner = 2 * np.log2(procs) * eager_alpha + 2 * ((procs-1)/procs) * n * (ppn/eager_R_N)
        
        print(f"For message size: 2^{j} \tRecursive Doubling: {t_recursive_doubling}\t\tBinomial Tree Algorithm: {t_binom_tree}\t\tRabenseifner's Algorithm: {t_rabenseifner}")
    
    for j in range(10):
        n = 2**(j+11) * 8
        t_recursive_doubling = np.log2(procs) * (rend_alpha +  n * (ppn/rend_R_N))
        t_binom_tree = 2 * np.log2(procs) * rend_alpha + 2 * np.log2(procs) * n * (ppn/rend_R_N)
        t_rabenseifner = 2 * np.log2(procs) * rend_alpha + 2 * ((procs-1)/procs) * n * (ppn/rend_R_N)
        
        print(f"For message size: 2^{j+11} \tRecursive Doubling: {t_recursive_doubling}\t\tBinomial Tree Algorithm: {t_binom_tree}\t\tRabenseifner's Algorithm: {t_rabenseifner}")