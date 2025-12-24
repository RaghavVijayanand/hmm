#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 5          // number of states
#define M 32         // codebook size
#define MAX_T 500
#define MAX_SEQ 500
#define DIGITS 4
#define EPS 1e-6

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
        hmm->pi[i] = (i == 0) ? 1.0 : 0.0;

        for (int j = 0; j < N; j++)
        {
            if (i == j)
                hmm->A[i][j] = 0.6;
            else if (j == i + 1)
                hmm->A[i][j] = 0.4;
            else
                hmm->A[i][j] = 0.0;
        }

        for (int k = 0; k < M; k++)
            hmm->B[i][k] = 1.0 / M;
    }

    hmm->A[N-1][N-1] = 1.0;
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
        while (tok && s->T < MAX_T)
        {
            s->O[s->T++] = atoi(tok);
            tok = strtok(NULL, " \n");
        }
        count++;
    }
    fclose(fp);
    return count;
}

// ---------------- SCALED FORWARD ----------------

double forward_scaled(HMM *hmm, Sequence *seq)
{
    static double alpha[MAX_T][N];
    static double c[MAX_T];

    int T = seq->T;
    double logprob = 0;

    c[0] = 0;
    for (int i = 0; i < N; i++)
    {
        alpha[0][i] = hmm->pi[i] * hmm->B[i][seq->O[0]];
        c[0] += alpha[0][i];
    }
    c[0] = 1.0 / (c[0] + EPS);
    for (int i = 0; i < N; i++)
        alpha[0][i] *= c[0];

    for (int t = 1; t < T; t++)
    {
        c[t] = 0;
        for (int j = 0; j < N; j++)
        {
            alpha[t][j] = 0;
            for (int i = 0; i < N; i++)
                alpha[t][j] += alpha[t-1][i] * hmm->A[i][j];
            alpha[t][j] *= hmm->B[j][seq->O[t]];
            c[t] += alpha[t][j];
        }
        c[t] = 1.0 / (c[t] + EPS);
        for (int j = 0; j < N; j++)
            alpha[t][j] *= c[t];
    }

    for (int t = 0; t < T; t++)
        logprob -= log(c[t]);

    return logprob;
}

// ---------------- BAUM-WELCH TRAINING ----------------

void baum_welch(HMM *hmm, Sequence *seqs, int S, int iters)
{
    static double alpha[MAX_T][N], beta[MAX_T][N], gamma[MAX_T][N];
    static double xi[MAX_T][N][N], c[MAX_T];

    for (int it = 0; it < iters; it++)
    {
        double pi_acc[N] = {0};
        double A_num[N][N] = {{0}}, A_den[N] = {0};
        double B_num[N][M] = {{0}}, B_den[N] = {0};

        for (int s = 0; s < S; s++)
        {
            Sequence *seq = &seqs[s];
            int T = seq->T;

            // ---- Forward (scaled) ----
            c[0] = 0;
            for (int i = 0; i < N; i++)
            {
                alpha[0][i] = hmm->pi[i] * hmm->B[i][seq->O[0]];
                c[0] += alpha[0][i];
            }
            c[0] = 1.0 / (c[0] + EPS);
            for (int i = 0; i < N; i++)
                alpha[0][i] *= c[0];

            for (int t = 1; t < T; t++)
            {
                c[t] = 0;
                for (int j = 0; j < N; j++)
                {
                    alpha[t][j] = 0;
                    for (int i = 0; i < N; i++)
                        alpha[t][j] += alpha[t-1][i] * hmm->A[i][j];
                    alpha[t][j] *= hmm->B[j][seq->O[t]];
                    c[t] += alpha[t][j];
                }
                c[t] = 1.0 / (c[t] + EPS);
                for (int j = 0; j < N; j++)
                    alpha[t][j] *= c[t];
            }

            // ---- Backward (scaled) ----
            for (int i = 0; i < N; i++)
                beta[T-1][i] = c[T-1];

            for (int t = T-2; t >= 0; t--)
                for (int i = 0; i < N; i++)
                {
                    beta[t][i] = 0;
                    for (int j = 0; j < N; j++)
                        beta[t][i] += hmm->A[i][j] *
                                      hmm->B[j][seq->O[t+1]] *
                                      beta[t+1][j];
                    beta[t][i] *= c[t];
                }

            // ---- Gamma & Xi ----
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
                             beta[t+1][j]) / (denom + EPS);
                        gamma[t][i] += xi[t][i][j];
                    }
                }
            }

            // last gamma
            double denom = 0;
            for (int i = 0; i < N; i++)
                denom += alpha[T-1][i] * beta[T-1][i];

            for (int i = 0; i < N; i++)
                gamma[T-1][i] =
                    (alpha[T-1][i] * beta[T-1][i]) / (denom + EPS);

            // ---- Accumulate ----
            for (int i = 0; i < N; i++)
                pi_acc[i] += gamma[0][i];

            for (int i = 0; i < N; i++)
                for (int t = 0; t < T-1; t++)
                    A_den[i] += gamma[t][i];

            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    for (int t = 0; t < T-1; t++)
                        A_num[i][j] += xi[t][i][j];

            for (int i = 0; i < N; i++)
                for (int t = 0; t < T; t++)
                {
                    B_den[i] += gamma[t][i];
                    B_num[i][seq->O[t]] += gamma[t][i];
                }
        }

        // ---- Re-estimation ----
        for (int i = 0; i < N; i++)
            hmm->pi[i] = pi_acc[i] / S;

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                hmm->A[i][j] = A_num[i][j] / (A_den[i] + EPS);

        for (int i = 0; i < N; i++)
            for (int k = 0; k < M; k++)
                hmm->B[i][k] =
                    (B_num[i][k] + EPS) / (B_den[i] + M * EPS);
    }
}

// ---------------- MAIN ----------------

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
        baum_welch(&models[d], train, S, 25);

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
                double p = forward_scaled(&models[m], &test[s]);
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
