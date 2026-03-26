# Risk Assessment Document

## DisassembleImage GPU Acceleration Project

**Document Version:** 1.0
**Date:** 2026-03-26
**Phase:** 05 - Research & Technology Selection

---

## Executive Summary

This document identifies technical risks associated with integrating OpenCL GPU acceleration and Dear ImGui into the existing DisassembleImage codebase. The project involves significant technology changes including GPU compute, real-time GUI frameworks, and cross-platform build complexity.

**Overall Risk Rating:** MEDIUM-HIGH
**Recommendation:** PROCEED WITH CAUTION - Mitigation strategies must be implemented before Phase 40 (GPU Core)

---

## Risk Register

### RISK-01: OpenCL Driver Unavailability or Instability

**Category:** OpenCL Driver Issues
**Severity:** High
**Likelihood:** Medium

**Impact:**
Application fails to start or crashes when OpenCL drivers are missing, outdated, or buggy. Users with NVIDIA GPUs may have CUDA but not OpenCL properly configured. AMD drivers on Linux are known to have OpenCL issues. Intel integrated graphics may lack OpenCL 1.2+ support entirely.

**Mitigation Strategy:**
- Implement graceful CPU fallback during application initialization
- Add runtime OpenCL capability detection before attempting GPU operations
- Provide clear error messages directing users to driver installation
- Test on minimum 3 different GPU vendors during prototype phase
- Document supported driver versions in README

**Contingency Plan:**
If OpenCL proves unreliable across target hardware, pivot to:
1. CPU-only mode as primary path (maintain current performance baseline)
2. Evaluate Vulkan Compute as alternative GPU API
3. Consider OpenMP/SIMD optimizations for CPU path

**Owner:** GPU Core Developer
**Monitoring:** Weekly hardware compatibility tests during Phase 40-50

---

### RISK-02: Memory Transfer Overhead Negates GPU Speedup

**Category:** Performance Risk
**Severity:** High
**Likelihood:** High

**Impact:**
For small images (typical OBJ face textures are 512x512 to 2048x2048), the time spent transferring data between CPU and GPU memory may exceed the actual processing time. This would result in GPU mode being slower than optimized CPU code, making the complexity unjustified.

**Mitigation Strategy:**
- Implement pinned memory (CL_MEM_ALLOC_HOST_PTR) for faster transfers
- Batch multiple face processing operations into single GPU command queue
- Use OpenCL-OpenGL interop to avoid readback for display (zero-copy to ImGui)
- Set minimum image size threshold (e.g., 1024x1024) below which CPU is used
- Profile actual transfer vs compute time in prototype phase

**Contingency Plan:**
If GPU speedup is <2x for typical workloads:
1. Disable GPU path for images below threshold size
2. Focus optimization efforts on CPU SIMD path instead
3. Document GPU mode as "large image only" feature

**Owner:** Performance Engineer
**Monitoring:** Benchmark suite must show 5x+ speedup on 2048x2048 images before Phase 50

---

### RISK-03: GPU Memory Exhaustion on Large Images

**Category:** Image Size Limits
**Severity:** High
**Likelihood:** Medium

**Impact:**
High-resolution source images (8K textures, 8192x8192) require 256MB+ per buffer. With multiple intermediate buffers (input, mask, output, temp), a complex disassembly could exceed 1GB GPU memory. This causes CL_MEM_OBJECT_ALLOCATION_FAILURE crashes.

**Mitigation Strategy:**
- Query CL_DEVICE_GLOBAL_MEM_SIZE at startup and set processing limits
- Implement tiled processing for large images (process in chunks)
- Use single-pass kernels where possible to reduce intermediate buffers
- Add memory budget calculator: warn user if operation will exceed available VRAM
- Implement streaming processing: process one face at a time, not entire image

**Contingency Plan:**
If memory limits cannot be managed:
1. Implement automatic CPU fallback for images exceeding GPU memory
2. Add "Maximum GPU Resolution" user setting with safe defaults
3. Document memory requirements prominently

**Owner:** GPU Core Developer
**Monitoring:** Memory stress tests with 4K, 8K, and 16K images during Phase 40

---

### RISK-04: CMake Build System Complexity

**Category:** Build Complexity
**Severity:** Medium
**Likelihood:** High

**Impact:**
The project requires: OpenCL (system library), Dear ImGui (FetchContent), GLFW (FetchContent), GLAD (FetchContent), OpenCV (system library), imgui-filebrowser (FetchContent). Version conflicts between these dependencies, especially OpenGL context handling between OpenCL interop and GLFW, can cause build failures that are difficult to diagnose.

**Mitigation Strategy:**
- Create modular CMake structure: core library, GPU module, GUI module
- Pin exact versions for all FetchContent dependencies
- Provide Docker-based build environment for reproducible builds
- Create separate CMake presets for Windows (Visual Studio) and Linux (GCC/Clang)
- Add CMake configure-time checks for OpenCL/OpenGL compatibility

**Contingency Plan:**
If build system becomes unmaintainable:
1. Simplify by removing OpenCL-OpenGL interop (accept performance penalty)
2. Use pre-built dependencies instead of FetchContent
3. Split into separate GPU daemon process with simpler IPC

**Owner:** Build System Maintainer
**Monitoring:** Clean build must complete in <5 minutes on reference hardware

---

### RISK-05: OpenCL-OpenGL Context Sharing Failures

**Category:** Cross-Platform Compatibility
**Severity:** Medium
**Likelihood:** Medium

**Impact:**
The planned zero-copy texture display using OpenCL-OpenGL interop requires proper context sharing setup. This is platform-specific: WGL on Windows, GLX on Linux, CGL on macOS. Context creation order matters, and errors manifest as black screens or segfaults with cryptic driver errors.

**Mitigation Strategy:**
- Abstract context sharing into platform-specific classes
- Implement fallback path: GPU processes to buffer, CPU uploads to OpenGL texture
- Use CL/GL interop only after explicit capability check
- Test context creation on Windows 10/11 and Ubuntu 22.04 minimum
- Add detailed logging for context creation steps

**Contingency Plan:**
If interop fails on target platforms:
1. Use clEnqueueReadBuffer + glTexSubImage2D (slight overhead but reliable)
2. Implement double-buffered CPU-GPU transfer to hide latency
3. Document interop as "experimental" feature

**Owner:** Graphics Integration Developer
**Monitoring:** Context creation tests in CI for both Windows and Linux

---

### RISK-06: Existing Fork-Based Code Conflicts with GPU Threading

**Category:** Integration Risk
**Severity:** High
**Likelihood:** High

**Impact:**
The existing codebase uses `fork()` without `exec()` to create worker processes. OpenCL contexts are not fork-safe; forking after OpenCL initialization causes undefined behavior including crashes and device loss. The current multi-process architecture cannot coexist with GPU compute.

**Mitigation Strategy:**
- Refactor `player_settings_factory` to use threads instead of processes
- Implement proper thread pool for CPU fallback path
- Use mutex/condition variable synchronization instead of process boundaries
- Ensure OpenCL context is created AFTER process fork (if fork retained)
- Add architecture decision record documenting threading model change

**Contingency Plan:**
If fork cannot be removed (e.g., for isolation requirements):
1. Create GPU process that communicates via shared memory/IPC
2. Fork before OpenCL initialization, use IPC to submit GPU jobs
3. Accept complexity of multi-process GPU architecture

**Owner:** Architecture Lead
**Monitoring:** Thread safety audit of all shared data structures before Phase 40

---

### RISK-07: OpenCL Kernel Compilation Failures

**Category:** OpenCL Driver Issues
**Severity:** Medium
**Likelihood:** Medium

**Impact:**
OpenCL kernels are compiled at runtime by the driver. Different drivers support different language versions and extensions. A kernel that compiles on NVIDIA may fail on Intel or AMD due to: strict vector type casting, missing extensions, different math precision defaults, or driver bugs.

**Mitigation Strategy:**
- Target OpenCL 1.2 as baseline (widest support)
- Avoid vendor-specific extensions unless guarded by #ifdef
- Implement kernel compilation caching to disk (avoid recompile on each run)
- Add comprehensive build log capture and user-friendly error reporting
- Test kernel compilation on Intel, AMD, and NVIDIA hardware

**Contingency Plan:**
If kernel portability fails:
1. Maintain separate kernel versions per vendor (maintenance burden)
2. Implement SPIR-V binary distribution (requires OpenCL 2.1+)
3. Fall back to CPU implementation for failing devices

**Owner:** GPU Core Developer
**Monitoring:** Kernel compilation test suite across vendor hardware

---

### RISK-08: Dear ImGui Learning Curve and Integration Complexity

**Category:** Integration Risk
**Severity:** Medium
**Likelihood:** Medium

**Impact:**
Dear ImGui uses immediate-mode GUI paradigm which is fundamentally different from retained-mode frameworks. Developers unfamiliar with this pattern may create: frame rate dependent UI logic, incorrect widget state handling, memory leaks from unclosed Begin/End blocks, or improper input handling. The docking branch adds additional complexity.

**Mitigation Strategy:**
- Create UI component library with documented patterns
- Implement RAII wrappers for ImGui Begin/End pairs
- Use ImGui demo window as reference during development
- Establish code review checklist for UI changes
- Create style guide for consistent ImGui usage patterns

**Contingency Plan:**
If ImGui proves too complex:
1. Evaluate lightweight alternatives (nuklear, microui)
2. Consider keeping CLI interface as primary UI
3. Use native file dialogs via nfd library instead of ImGui file browser

**Owner:** GUI Developer
**Monitoring:** UI code review feedback and bug reports

---

### RISK-09: Windows Path Separator and Encoding Issues

**Category:** Cross-Platform Compatibility
**Severity:** Low
**Severity:** Medium

**Impact:**
The existing codebase uses hardcoded `/` path separators and Chinese character output. Windows uses `\` separators and may have encoding issues with Chinese console output. File dialogs may return paths with mixed separators causing file open failures.

**Mitigation Strategy:**
- Use `std::filesystem::path` (C++17) for all path operations
- Normalize paths immediately after receiving from file dialog
- Set console codepage to UTF-8 on Windows startup
- Test file operations with paths containing spaces and Unicode
- Use platform-independent path concatenation

**Contingency Plan:**
If cross-platform issues persist:
1. Document Windows as "community supported" platform
2. Focus primary development on Linux, accept patches for Windows
3. Use cross-platform file dialog library (nfd) instead of native

**Owner:** Platform Maintainer
**Monitoring:** CI builds on Windows and Linux with file operation tests

---

### RISK-10: Existing Memory Leaks Compound with GPU Resources

**Category:** Integration Risk
**Severity:** Medium
**Likelihood:** High

**Impact:**
The existing codebase has documented memory leaks (raw pointers without delete). GPU resources (cl::Buffer, cl::Image2D, OpenGL textures) also require explicit cleanup. If the existing leak patterns are replicated in GPU code, the application will exhaust both system RAM and GPU VRAM during long batch processing sessions.

**Mitigation Strategy:**
- Use RAII wrappers (cl::Buffer, cl::Image2D from C++ bindings) exclusively
- Implement GPU resource pool with automatic cleanup
- Add valgrind/address sanitizer CI checks for memory leaks
- Refactor existing leak-prone code before adding GPU features
- Implement application shutdown cleanup verification

**Contingency Plan:**
If memory issues cannot be resolved:
1. Document application as "restart recommended between batches"
2. Implement periodic resource cleanup checkpoints
3. Add memory monitoring with automatic graceful degradation

**Owner:** Code Quality Lead
**Monitoring:** Valgrind leak checks in CI, GPU memory profiling

---

### RISK-11: Small Image Kernel Launch Overhead

**Category:** Performance Risk
**Severity:** Medium
**Likelihood:** High

**Impact:**
OBJ face textures can be as small as 128x128 pixels. GPU kernel launch overhead (command queue submission, context switching) can be 0.1-1ms. For small images, this overhead may exceed the actual processing time, making GPU slower than CPU for typical workloads.

**Mitigation Strategy:**
- Implement batch processing: process multiple faces in single kernel launch
- Use CPU path for images below configurable threshold (default 512x512)
- Profile kernel launch vs compute time for various image sizes
- Consider kernel fusion: combine multiple operations into single kernel
- Use persistent worker threads to amortize launch cost

**Contingency Plan:**
If overhead cannot be reduced:
1. Document GPU mode as "large batch/large image only" feature
2. Implement automatic CPU/GPU selection based on workload characteristics
3. Focus on throughput optimization for batch processing

**Owner:** Performance Engineer
**Monitoring:** Benchmark results showing kernel launch time vs compute time ratio

---

### RISK-12: OpenCV Version Compatibility

**Category:** Build Complexity
**Severity:** Low
**Likelihood:** Medium

**Impact:**
The project uses OpenCV for image I/O and basic operations. OpenCV 3.x vs 4.x have API differences (cv::IMREAD vs CV_LOAD_IMAGE). System-installed OpenCV versions vary across distributions. GPU-accelerated OpenCV (cv::UMat) may conflict with custom OpenCL code.

**Mitigation Strategy:**
- Specify minimum OpenCV version (4.0+) in CMake
- Use OpenCV C++ API consistently (avoid deprecated C API)
- Isolate OpenCV usage behind abstraction layer
- Test with OpenCV built with and without OpenCL support
- Document required OpenCV modules: core, imgproc, imgcodecs

**Contingency Plan:**
If OpenCV compatibility issues arise:
1. Vendor specific OpenCV version via FetchContent (build from source)
2. Replace OpenCV with stb_image for I/O, custom code for transforms
3. Support multiple OpenCV versions with #ifdef guards

**Owner:** Build System Maintainer
**Monitoring:** CI builds with different OpenCV versions

---

## Risk Matrix Summary

| Risk ID | Description | Severity | Likelihood | Priority |
|---------|-------------|----------|------------|----------|
| RISK-01 | OpenCL Driver Issues | High | Medium | 1 |
| RISK-02 | Memory Transfer Overhead | High | High | 1 |
| RISK-03 | GPU Memory Exhaustion | High | Medium | 1 |
| RISK-06 | Fork/Threading Conflict | High | High | 1 |
| RISK-04 | CMake Build Complexity | Medium | High | 2 |
| RISK-05 | CL/GL Context Sharing | Medium | Medium | 2 |
| RISK-07 | Kernel Compilation Failures | Medium | Medium | 2 |
| RISK-08 | ImGui Learning Curve | Medium | Medium | 2 |
| RISK-10 | Memory Leaks + GPU | Medium | High | 2 |
| RISK-11 | Small Image Overhead | Medium | High | 2 |
| RISK-09 | Windows Path Issues | Low | Medium | 3 |
| RISK-12 | OpenCV Compatibility | Low | Medium | 3 |

---

## Risk Monitoring Approach

### Continuous Monitoring (Ongoing)

1. **Build Health Dashboard**
   - Track build success rate across platforms
   - Monitor build time trends
   - Alert on dependency fetch failures

2. **Performance Regression Tests**
   - Daily benchmark runs on reference hardware
   - Track GPU speedup ratio vs CPU baseline
   - Alert if speedup drops below 3x

3. **Hardware Compatibility Matrix**
   - Maintain tested GPU/driver list
   - Track reported issues by hardware configuration
   - Update minimum requirements quarterly

### Phase Gate Checkpoints

**Before Phase 40 (GPU Core):**
- [ ] OpenCL prototype runs on 3+ different GPU vendors
- [ ] CPU fallback path fully functional
- [ ] Fork-based code refactored to threads
- [ ] Memory leak fixes backported to existing code

**Before Phase 50 (GUI Integration):**
- [ ] OpenCL-OpenGL interop tested on target platforms
- [ ] ImGui prototype achieves 60+ FPS
- [ ] File dialog works on Windows and Linux
- [ ] GPU memory stress tests pass

**Before Phase 90 (Release):**
- [ ] 95% kernel compilation success rate across test hardware
- [ ] Automatic CPU fallback verified for all failure modes
- [ ] No critical or high severity memory leaks
- [ ] Documentation complete for all known limitations

---

## Go/No-Go Decision

### Current Assessment: GO WITH MITIGATIONS

The project should proceed to Phase 40 (GPU Core) with the following conditions:

### Required Before Phase 40

1. **RISK-06 (Fork/Threading)** - MUST be resolved. The existing fork-based code must be refactored to use threads before any OpenCL code is integrated.

2. **RISK-02 (Memory Transfer)** - Prototype must demonstrate measurable speedup (>2x) on representative image sizes (1024x1024 minimum).

3. **RISK-04 (Build Complexity)** - CMake must successfully configure and build on both Windows and Linux without manual intervention.

### Recommended Before Phase 40

4. **RISK-10 (Memory Leaks)** - Address existing memory leaks to prevent compounding with GPU resources.

5. **RISK-01 (Driver Issues)** - Implement CPU fallback and graceful degradation.

### Conditions That Trigger Reassessment

- Prototype fails to achieve 2x speedup on 2048x2048 images
- OpenCL kernel compilation fails on >20% of test hardware
- Build system requires >30 minutes clean build time
- Memory usage exceeds 2x baseline during stress testing
- Critical security vulnerabilities discovered in dependencies

### Contingency: Pivot to CPU Optimization

If GPU acceleration proves infeasible, the project will:
1. Retain Dear ImGui for improved UX
2. Implement OpenMP/SIMD optimizations for CPU path
3. Focus on algorithmic improvements (tiling, caching)
4. Document GPU as "future enhancement"

---

## Risk Acceptance Signatures

| Role | Name | Date | Decision |
|------|------|------|----------|
| Technical Lead | _________________ | ________ | GO / NO-GO |
| GPU Developer | _________________ | ________ | GO / NO-GO |
| QA Lead | _________________ | ________ | GO / NO-GO |

---

## Appendix A: Risk Scoring Methodology

**Severity Levels:**
- **High:** Application crash, data loss, security breach, or complete feature failure
- **Medium:** Performance degradation, feature limitation, or significant maintenance burden
- **Low:** Minor inconvenience, workaround available, or cosmetic issue

**Likelihood Levels:**
- **High:** >50% probability of occurrence during project lifetime
- **Medium:** 20-50% probability of occurrence
- **Low:** <20% probability of occurrence

**Priority Calculation:**
- Priority 1: High Severity (regardless of likelihood)
- Priority 2: Medium Severity + Medium/High Likelihood, or Low Severity + High Likelihood
- Priority 3: All other combinations

---

## Appendix B: Reference Hardware for Testing

### Minimum Target
- Intel HD Graphics 630 or equivalent
- 4GB system RAM
- OpenCL 1.2 support

### Recommended Target
- NVIDIA GTX 1060 6GB or AMD RX 580
- 8GB system RAM
- OpenCL 2.0 support

### High-End Target
- NVIDIA RTX 3060 or AMD RX 6700 XT
- 16GB system RAM
- OpenCL 3.0 support

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2026-03-26 | Phase 05 Executor | Initial risk assessment based on research findings |
