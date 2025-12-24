#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 5          // number of states
#define M 32         // number of symbols (codebook size)
#define MAX_T 500
#define MAX_SEQ 500
#define DIGITS 4     // 2,3,4,5

// ---------------- DATA STRUCTURES ----------------

typedef struct {
    int T;
    int O[MAX_T];
} Sequence;

typedef struct {
    double A[N][N];
    double B[N][M];
    double pi[N];
} HMM;

// ---------------- INITIALIZE HMM ----------------

void init_hmm(HMM *hmm)
{
    for (int i = 0; i < N; i++)
    {
        hmm->pi[i] = (i == 0);

        for (int j = 0; j < N; j++)
            hmm->A[i][j] = (j == i || j == i + 1) ? 0.5 : 0.0;

        for (int k = 0; k < M; k++)
            hmm->B[i][k] = 1.0 / M;
    }
}

// ---------------- LOAD SEQUENCES ----------------

int load_sequences(const char *file, Sequence *seqs)
{
    FILE *fp = fopen(file, "r");
    if (!fp) return 0;

    int count = 0;
    char line[4096];

    while (fgets(line, sizeof(line), fp))
    {
        Sequence *s = &seqs[count];
        s->T = 0;

        char *tok = strtok(line, " \n");
        while (tok)
        {
            s->O[s->T++] = atoi(tok);
            tok = strtok(NULL, " \n");
        }
        count++;
    }
    fclose(fp);
    return count;
}

// ---------------- FORWARD ALGORITHM ----------------

double forward(HMM *hmm, Sequence *seq)
{
    double alpha[MAX_T][N];
    int T = seq->T;

    for (int i = 0; i < N; i++)
        alpha[0][i] = hmm->pi[i] * hmm->B[i][seq->O[0]];

    for (int t = 1; t < T; t++)
        for (int j = 0; j < N; j++)
        {
            alpha[t][j] = 0;
            for (int i = 0; i < N; i++)
                alpha[t][j] += alpha[t-1][i] * hmm->A[i][j];
            alpha[t][j] *= hmm->B[j][seq->O[t]];
        }

    double prob = 0;
    for (int i = 0; i < N; i++)
        prob += alpha[T-1][i];

    return log(prob + 1e-12);
}

// ---------------- BAUM-WELCH TRAINING ----------------

void baum_welch(HMM *hmm, Sequence *seqs, int S, int iters)
{
    static double alpha[MAX_T][N], beta[MAX_T][N];
    static double gamma[MAX_T][N], xi[MAX_T][N][N];

    for (int it = 0; it < iters; it++)
    {
        for (int s = 0; s < S; s++)
        {
            Sequence *seq = &seqs[s];
            int T = seq->T;

            // Forward
            for (int i = 0; i < N; i++)
                alpha[0][i] = hmm->pi[i] * hmm->B[i][seq->O[0]];

            for (int t = 1; t < T; t++)
                for (int j = 0; j < N; j++)
                {
                    alpha[t][j] = 0;
                    for (int i = 0; i < N; i++)
                        alpha[t][j] += alpha[t-1][i] * hmm->A[i][j];
                    alpha[t][j] *= hmm->B[j][seq->O[t]];
                }

            // Backward
            for (int i = 0; i < N; i++)
                beta[T-1][i] = 1.0;

            for (int t = T-2; t >= 0; t--)
                for (int i = 0; i < N; i++)
                {
                    beta[t][i] = 0;
                    for (int j = 0; j < N; j++)
                        beta[t][i] += hmm->A[i][j] *
                                      hmm->B[j][seq->O[t+1]] *
                                      beta[t+1][j];
                }

            // Gamma & Xi
            for (int t = 0; t < T-1; t++)
            {
                double denom = 0;
                for (int i = 0; i < N; i++)
                    for (int j = 0; j < N; j++)
                        denom += alpha[t][i] * hmm->A[i][j] *
                                 hmm->B[j][seq->O[t+1]] * beta[t+1][j];

                for (int i = 0; i < N; i++)
                {
                    gamma[t][i] = 0;
                    for (int j = 0; j < N; j++)
                    {
                        xi[t][i][j] =
                            (alpha[t][i] * hmm->A[i][j] *
                             hmm->B[j][seq->O[t+1]] *
                             beta[t+1][j]) / (denom + 1e-12);
                        gamma[t][i] += xi[t][i][j];
                    }
                }
            }

            // Update pi
            for (int i = 0; i < N; i++)
                hmm->pi[i] = gamma[0][i];

            // Update A
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                {
                    double num = 0, den = 0;
                    for (int t = 0; t < T-1; t++)
                    {
                        num += xi[t][i][j];
                        den += gamma[t][i];
                    }
                    hmm->A[i][j] = num / (den + 1e-12);
                }

            // Update B
            for (int i = 0; i < N; i++)
                for (int k = 0; k < M; k++)
                {
                    double num = 0, den = 0;
                    for (int t = 0; t < T; t++)
                    {
                        if (seq->O[t] == k)
                            num += gamma[t][i];
                        den += gamma[t][i];
                    }
                    hmm->B[i][k] = num / (den + 1e-12);
                }
        }
    }
}

// ---------------- MAIN (TRAIN + TEST + ACCURACY) ----------------

int main()
{
    int labels[DIGITS] = {2,3,4,5};
    HMM models[DIGITS];

    // TRAINING
    for (int d = 0; d < DIGITS; d++)
    {
        char path[128];
        Sequence train[MAX_SEQ];

        sprintf(path, "hmm/%d/train.seq", labels[d]);
        int S = load_sequences(path, train);

        init_hmm(&models[d]);
        baum_welch(&models[d], train, S, 15);

        printf("Trained HMM for digit %d\n", labels[d]);
    }

    // TESTING
    int correct = 0, total = 0;

    for (int d = 0; d < DIGITS; d++)
    {
        char path[128];
        Sequence test[MAX_SEQ];

        sprintf(path, "hmm/%d/dev.seq", labels[d]);
        int S = load_sequences(path, test);

        for (int s = 0; s < S; s++)
        {
            double best = -1e18;
            int pred = -1;

            for (int m = 0; m < DIGITS; m++)
            {
                double p = forward(&models[m], &test[s]);
                if (p > best)
                {
                    best = p;
                    pred = labels[m];
                }
            }

            if (pred == labels[d])
                correct++;

            total++;
        }
    }

    printf("\nAccuracy = %.2f%% (%d / %d)\n",
           100.0 * correct / total, correct, total);

    return 0;
}
