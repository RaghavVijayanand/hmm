#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <sys/stat.h>

#define DIM 39
#define K 32
#define MAX_PATH 1024

typedef struct {
    double x[DIM];
} Vector;

Vector codebook[K];


// ---------------- Codebook ----------------
void load_codebook(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Cannot open codebook\n");
        exit(1);
    }

    for (int k = 0; k < K; k++)
        for (int d = 0; d < DIM; d++)
            fscanf(fp, "%lf", &codebook[k].x[d]);

    fclose(fp);
}

// ---------------- Distance ----------------
double squared_distance(Vector a, Vector b)
{
    double sum = 0.0;
    for (int i = 0; i < DIM; i++)
    {
        double diff = a.x[i] - b.x[i];
        sum += diff * diff;
    }
    return sum;
}

// ---------------- VQ ----------------
int vq_map(Vector frame)
{
    int best = 0;
    double best_dist = squared_distance(frame, codebook[0]);

    for (int k = 1; k < K; k++)
    {
        double d = squared_distance(frame, codebook[k]);
        if (d < best_dist)
        {
            best_dist = d;
            best = k;
        }
    }
    return best;
}

// ---------------- Class from filename ----------------
int get_class_from_name(const char *name)
{
    if (strstr(name, "2")) return 2;
    if (strstr(name, "3")) return 3;
    if (strstr(name, "4")) return 4;
    if (strstr(name, "5")) return 5;
    return -1;
}

// ---------------- Process folder ----------------
void process_folder(const char *mfcc_dir, const char *split)
{
    DIR *dir = opendir(mfcc_dir);
    struct dirent *entry;

    if (!dir)
    {
        printf("Cannot open %s\n", mfcc_dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;

        int cls = get_class_from_name(entry->d_name);
        if (cls == -1) continue;

        char mfcc_path[MAX_PATH];
        snprintf(mfcc_path, sizeof(mfcc_path),
                 "%s/%s", mfcc_dir, entry->d_name);

        char out_path[MAX_PATH];
        snprintf(out_path, sizeof(out_path),
                 "hmm/%d/%s.seq", cls, split);

        FILE *in = fopen(mfcc_path, "r");
        FILE *out = fopen(out_path, "a");

        if (!in || !out)
        {
            printf("File error: %s\n", entry->d_name);
            continue;
        }

        Vector frame;

        while (1)
        {
            for (int d = 0; d < DIM; d++)
            {
                if (fscanf(in, "%lf", &frame.x[d]) != 1)
                {
                    fprintf(out, "\n");
                    fclose(in);
                    fclose(out);
                    goto next_file;
                }
            }

            int symbol = vq_map(frame);
            fprintf(out, "%d ", symbol);
        }

    next_file:
        ;
    }

    closedir(dir);
}

// ---------------- Main ----------------
int main()
{
    load_codebook("codebook.txt");

    // create folders
    mkdir("hmm", 0777);
    mkdir("hmm/2", 0777);
    mkdir("hmm/3", 0777);
    mkdir("hmm/4", 0777);
    mkdir("hmm/5", 0777);

    process_folder("../../audios/combined_train", "train");
process_folder("../../audios/combined_dev", "dev");

    printf("VQ batching done. Sequences written to hmm/\n");
    return 0;
}
