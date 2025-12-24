# Docs for HMM Assignment

This `docs/` folder contains annotated source pages and a tutorial that explain the entire project flow from features to models.

Quick steps to view locally (PowerShell):

```pwsh
# from project root (where `docs/` is located)
# start a simple HTTP server and open docs
python -m http.server 8000
# open http://localhost:8000/docs/index.html in your browser
```

Quick compile/run (Windows/WSL notes):

```pwsh
# compile HMM program
gcc -o hmm.exe hmm.c -lm
# run (WSL recommended for vq_batch)
./hmm.exe
```

If you want me to: add an MFCC extraction example, modify K-means to output `codebook.txt`, or create a guided notebook with diagrams, tell me which next step you prefer.