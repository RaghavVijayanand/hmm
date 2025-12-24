#ifndef HMM_H
#define HMM_H

#define N 5          // states
#define M 32         // symbols (codebook size)
#define MAX_T 500
#define MAX_SEQ 500

typedef struct {
    int T;
    int O[MAX_T];
} Sequence;

typedef struct {
    double A[N][N];
    double B[N][M];
    double pi[N];
} HMM;

void init_hmm(HMM *hmm);
void baum_welch(HMM *hmm, Sequence *seqs, int n_seq, int iters);
double forward(HMM *hmm, Sequence *seq);

#endif
