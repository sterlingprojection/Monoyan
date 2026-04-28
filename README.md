# Monoyan 🌸

<p align="center">
  <img src="https://cdn.discordapp.com/attachments/1498437660690354176/1498438151155355820/monoyan_logo_1.png?ex=69f128c6&is=69efd746&hm=d705a512ecae17a531d59dae586ae2712739c3defe94432203a0a718941340b7&" alt="Monoyan Logo" width="200"/>
</p>

**Monoyan** is a high-performance, kernel-synchronized C++ Mono Injector. This standalone edition allows you to embed your DLL directly into the loader for a seamless, one-click experience.

## 🌸 The "Yan" Philosophy
"Yan" represents a sharp, low-level obsession with precision. We bypass high-level APIs in favor of direct kernel interaction and polymorphic memory streaming.

## 🛠️ Project Structure

### 1. Tool (Payload Generator)
The utility used to protect and prepare your DLLs.
- **Polymorphic Transformation**: Generates a unique encryption header (`payload.h`) for your DLL.
- **Recursive Math**: Implements a rolling-key algorithm to prevent static analysis.

### 2. Loader (Phantom Injector)
The core client that executes the injection.
- **Embedded Execution**: Reads the encrypted DLL directly from its own memory.
- **Kernel Sync**: Direct synchronization with `KUSER_SHARED_DATA` (0x7FFE0008).
- **Branded UI**: Clean, magenta-on-black console with a clickable Discord overlay.

## 🚀 Getting Started

### 1. Generate the Header
- Build the **Tool** project in Visual Studio.
- Drag your target DLL onto `Tool.exe`.
- A `payload.h` file will be generated in the same folder.

### 2. Embed the Payload
- Copy the generated `payload.h` into the `Loader/src/` directory, overwriting the existing placeholder.

### 3. Build the Loader
- Open `Monoyan.sln` in Visual Studio.
- Build the **Loader** project in `Release | x64` configuration.

### 4. Run
- Launch the compiled `Loader.exe`. It will automatically wait for the target process and inject the embedded payload.

---
*Stay Sharp. Stay Yan.*
