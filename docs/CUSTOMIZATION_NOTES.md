# Customization Complete ✅

## What Was Done

Created a comprehensive, **parameter-focused** learning resource tailored to **your exact code**. Every constant in your project now has detailed explanations of **why** it was chosen.

### New Page Created:
**`parameters.html`** — A dedicated page explaining every constant in your code:

- **DIM = 39** — Why MFCC uses 39 dimensions (13 base + 13 delta + 13 delta-delta)
- **K = 100 (kmeans)** — Why K-means uses 100 clusters for codebook exploration
- **K = 32 (vq_batch, hmm)** — Why final HMM codebook uses 32 symbols (compression vs fidelity tradeoff)
- **N = 5 (HMM states)** — Why 5 hidden states (phonetic phases: onset, articulation, nucleus, offset, coda)
- **M = 32 (HMM symbols)** — Why 32 symbols (matches VQ codebook, avoids overfitting on small digit dataset)
- **MAX_ITER = 100** — Why 100 K-means iterations (safe convergence bound)
- **EPS = 1e-4** — Why this convergence threshold (standard quality/efficiency balance)
- **MAX_POINTS, MAX_T, MAX_SEQ** — Buffer sizes explained
- **DIGITS = 4** — Why 4 classes (digits 2, 3, 4, 5)
- **Baum–Welch iterations = 15** — Why 15 EM iterations (empirical convergence for digit HMMs)

**Plus**: Summary table, reasoning for each choice, and "How to Experiment" section.

### Pages Updated:
1. **`fundamentals.html`** — Now references actual project parameters and links to parameters page
2. **`math_appendix.html`** — Worked example changed from generic 2-state to your actual 5-state, 32-symbol HMM
3. **`getting_started.html`** — Step 2 now highlights Parameters Explained (marked ⭐)
4. **`index.html`** — Linked Parameters page prominently in learning path

## Learning Path Now Emphasizes "Why"

Recommended progression for understanding **your code**:
1. **Fundamentals** — High-level concepts
2. **⭐ Parameters Explained** ← NEW: Tailored to your DIM=39, N=5, M=32, K=32 choices
3. **Glossary** — Terminology
4. **Overview** — File descriptions
5. **Tutorial** — Commands to run
6. **Source Code** — Line-by-line annotations
7. **Math Appendix** — Detailed equations with your actual parameter dimensions

## Key Features

✅ **Extracted from your actual source code:**
- DIM from kmeans.c line 5
- K values from kmeans.c (100) and vq_batch.c (32)
- N, M, DIGITS from hmm.c lines 6–10
- Baum–Welch iterations from hmm.c line 209

✅ **Parameter rationale tailored to digit recognition:**
- Explains why 5 states (not 3 or 10) for digit phonetics
- Explains why 32 vs 100 clusters (compression tradeoff for small dataset)
- Explains why 39-D MFCC (industry standard for speech)

✅ **"Why this value here?" answered clearly:**
- Each parameter box has:
  - The value from your code
  - File reference (e.g., "kmeans.c line 6")
  - Detailed "Why?" explanation
  - Context for digit recognition task

## Next Steps

**Users visiting your website will now see:**
1. **Getting Started** → immediately encounters link to **Parameters Explained** as Step 2
2. **Learn fundamentals** → fundamentals.html now says "See Parameters for your exact choices"
3. **Read code** → know exactly why those constants were chosen
4. **Math deep-dive** → appendix uses 5-state, 32-symbol examples (not generic 2-state)

**All original content preserved** — just enriched with parameter context throughout.

---

**Status**: ✅ Complete. All 6 learning pages + code annotations now fully customized to your project's parameters.
