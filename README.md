Hybrid Image Compression using Daubechies DWT (db4) + Huffman Coding
📌 Introduction
This project implements a hybrid image compression prototype for hyperspectral imagery using two key techniques:

1. Daubechies Wavelet Transform (DWT) - specifically the db4 filter
2. Real Huffman Coding

The goal is to efficiently compress 3 selected bands from a hyperspectral dataset and reconstruct them with minimal loss of quality, as measured by MSE, PSNR, and SSIM.
🧰 Tools & Environment
- Language: C++
- Compiler: g++ (MinGW64)
- Libraries: OpenCV (for image I/O), C++ STL
- Platform: Windows 10 / MSYS2 / Git Bash

🔁 Compression Flow Overview
1. **Data Input:**
   - Three `.bin` files (band_0.bin, band_1.bin, band_2.bin) with shape (145, 145).
   - Automatically detects image dimensions.

2. **Preprocessing:**
   - Pads the image to ensure even dimensions for wavelet transform.

3. **2-Level Daubechies DWT (db4):**
   - Apply Level-1 db4 DWT → LL1
   - Apply Level-2 db4 DWT on LL1 → LL2 (final compressed data)

4. **Flatten & Compress:**
   - Flatten LL2 into a 1D array
   - Apply Real Huffman Coding to compress the data

5. **Decompression & Reconstruction:**
   - Huffman decoding
   - Inverse DWT Level 2: LL2 → LL1 (with LH2, HL2, HH2 zeroed)
   - Inverse DWT Level 1: LL1 → Reconstructed image (with LH1, HL1, HH1 zeroed)

6. **Postprocessing:**
   - Crop reconstructed image to original size (145x145)

7. **Evaluation:**
   - Compute MSE, PSNR, SSIM
   - Output compressed size, CR (Compression Ratio), BPP (Bits Per Pixel)

8. **Save Output:**
   - Save reconstructed image (color) and compressed `.bin` files.
✨ Features
- Daubechies db4 transform for better frequency localization
- 2-level compression for higher CR
- Huffman coding with custom Huffman table generation
- Works for any image size (auto-detection + crop)
- Final image output in PNG format

🧪 Output Example (Console Log)
[1] Loading raw hyperspectral bands...

=== Processing Channel 0 ===
[2] Applying Level 1 DWT (db4)...
[2] Applying Level 2 DWT (db4)...
[3] Flattening + Real Huffman Encoding...
[4] Reconstructing...
[5] Evaluating...
MSE: 187.323
PSNR: 25.41 dB
SSIM: 0.872
Compression Ratio: 4.33
BPP: 1.72

[6] Saving full color output...
✅ DONE! Output saved to output/reconstructed_image.png

---

Created for research: Hybrid Hyperspectral Image Compression using Daubechies DWT (db4) and Huffman Coding.
