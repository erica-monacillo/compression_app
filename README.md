# Hybrid Hyperspectral Image Compression using Daubechies DWT (db4) + Huffman Coding

This project implements a hybrid compression prototype using:
- **Daubechies Wavelet Transform (db4)** for multi-level decomposition
- **Real Huffman Coding** for entropy-based lossless compression

It is designed for hyperspectral data (e.g., 145×145×200 bands) but currently focuses on compressing the first 3 bands (RGB).

---

## Tools & Environment

| Tool           | Description               |
|----------------|---------------------------|
| C++            | Core language             |
| OpenCV         | Image loading/saving      |
| MinGW + MSYS2  | Windows build toolchain   |
| Git            | Version control           |

---

## Compression Flow

```text
              ┌──────────────────────────┐
              │  Input .bin (3 bands)    │
              └──────────┬───────────────┘
                         │
              For Each Band (R/G/B)
                         ▼
         ┌────────────────────────────────┐
         │  1. Level-1 DWT (db4)          │
         │  2. Level-2 DWT (db4 on LL1)   │
         └──────────┬─────────────────────┘
                    ▼
         ┌───────────────────────────────┐
         │  3. Huffman Encoding (LL2)    │
         └──────────┬────────────────────┘
                    ▼
         ┌───────────────────────────────┐
         │  4. Huffman Decoding (LL2)    │
         └──────────┬────────────────────┘
                    ▼
         ┌───────────────────────────────┐
         │  5. IDWT Level-2 (LL2 → LL1)  │
         │  6. IDWT Level-1 (LL1 → Image)│
         └──────────┬────────────────────┘
                    ▼
        ┌────────────────────────────────┐
        │  7. Evaluation (PSNR, SSIM)    │
        │  8. Save Output Image (.png)   │
        └────────────────────────────────┘


 Features
Fully working DWT db4 decomposition (level 1 & level 2)

 Real Huffman encoding/decoding using frequency tables

 Automatic .bin image size detection

 Crops reconstructed output to match original size

 Outputs evaluation metrics (MSE, PSNR, SSIM, CR, BPP)

 Sample Console Output

$ ./compressApp
[1] Loading raw hyperspectral bands...

=== Processing Channel 0 ===
[2] Applying Level 1 DWT (db4)...
[2] Applying Level 2 DWT (db4)...
[3] Flattening + Real Huffman Encoding...
[4] Reconstructing...
[5] Evaluating...
MSE: 872.2
PSNR: 18.7 dB
SSIM: 0.61
CR: 5.45
BPP: 1.45
 DONE! Output saved to output/reconstructed_image.png


 Folder Structure
compression_app/
├── data/                   # .bin hyperspectral inputs
├── output/                 # Encoded & reconstructed outputs
├── include/                # Header files (.hpp)
├── src/                    # Source code (.cpp)
│   ├── main.cpp
│   ├── dwt_db4.cpp
│   ├── huffman.cpp
│   ├── image_io.cpp
│   ├── utils.cpp
├── README.md


Evaluation Metrics
MSE (Mean Squared Error)

PSNR (Peak Signal-to-Noise Ratio)

SSIM (Structural Similarity)

CR (Compression Ratio)

BPP (Bits Per Pixel)



---

 Just copy everything inside the **code block above** and paste it into your `README.md` file on GitHub.

If you want, I can also generate the actual file (`README.md`) for download or auto-upload to your repo.





