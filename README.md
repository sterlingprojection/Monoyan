# Monoyan 🌸

**Monoyan** is a high-performance, kernel-synchronized C++ Mono Injector solution. It utilizes a dual-project architecture to provide secure, fileless, and polymorphic DLL delivery.

## 🌸 The "Yan" Philosophy
"Yan" represents a sharp, low-level obsession with precision. We bypass high-level APIs in favor of direct kernel interaction and custom memory streaming.

## 🛠️ Project Structure

### 1. Tool (Payload Generator)
The utility used to protect and prepare your DLLs for the loader.
- **Linguistic Obfuscation**: Encodes data into a randomized Chinese character script.
- **Polymorphic Transformation**: Generates unique encryption tables for every payload.
- **Recursive Math**: Implements a rolling-key algorithm to prevent static analysis.

### 2. Loader (Phantom Injector)
The core client that executes the injection.
- **Phantom Streaming**: Receives data in fragments and writes directly to the target process.
- **Kernel Sync**: Direct synchronization with `KUSER_SHARED_DATA` (0x7FFE0008).
- **Hardware Entropy**: Ultra-fast title flickering driven by `__rdtsc`.
- **Stealth UI**: Clean, branded console with a clickable Discord overlay.

## 🚀 Getting Started

### 1. Prepare the Payload
- Build the **Tool** project in Visual Studio.
- Drag your target DLL onto `Tool.exe`.
- A `payload.yan` file will be generated.

### 2. Host the Stream
- Host the `payload.yan` on a raw TCP stream server.

### 3. Configure the Loader
- Open `Loader/src/main.cpp`.
- Update the `host` and `port` constants to match your server.
- Build the **Loader** project in Visual Studio.

### 4. Run
- Launch `Loader.exe`. It will automatically wait for the target process, stream the payload, and synchronize the core.

---
*Stay Sharp. Stay Yan.*
