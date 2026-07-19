# UE4 Mobile In-Process Overlay — Anti-Cheat Research Sample (Educational & Research Purpose Only)

![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)

> English ｜ [繁體中文](README_ZH.md)

This repository contains a conceptual sample demonstrating implementation techniques for in-process function hooking and overlay rendering inside an Unreal Engine 4 mobile application on iOS. The project is shared strictly for educational purposes, anti-cheat research, and reverse-engineering study.

## Important Disclaimer

**CRITICAL: THIS PROJECT IS FOR EDUCATIONAL AND RESEARCH PURPOSES ONLY.**

* This software exists to help security researchers understand how in-process cheats hook a game engine, intercept gameplay data, and render overlays — so that **anti-cheat countermeasures** can be designed against these exact techniques.
* Do not use this code in live, public, or production environments.
* Any deployment to gain an unfair advantage in matchmaking violates the terms of service of the respective game title and may be illegal under local jurisdictions.
* The author(s) assume absolutely no liability for misuse, account bans, legal consequences, or damages resulting from the use or modification of this source code.

---

## Target & Delivery

* **Platform:** iOS (arm64), jailbroken device required.
* **Target process:** a UE4-based mobile battle-royale title.
* **Injection vector:** the payload is built as a dynamic library that **masquerades as a `libwebp` framework**, so it is loaded by the host process at launch. The disguise must re-export the genuine WebP symbols or the host crashes.
* **Rendering:** same-process overlay drawn with Dear ImGui on top of the host's **Metal** layer (no separate window, no external read-process model).

---

## Features Matrix

| Feature | Technical Description | Research Focus |
| :--- | :--- | :--- |
| **Hide Record** | Keeps the overlay invisible to screen recording, screenshots, and streaming while staying visible on-device. | Exploits iOS `UITextField.secureTextEntry`: its backing layer is excluded from system capture, so the overlay is re-parented into that secure layer (`applyHideRecord`). |
| **ESP (Extra Sensory Perception)** | Draws bounding boxes, bone skeletons, and distance/name text over players. | UE4 entity-list traversal, bone matrix extraction, and 3D→2D World-to-Screen projection math. |
| **AIM (Angle)** | Adjusts camera/aim toward the nearest valid target by writing rotation vectors. | Pointer chains, vector math for pitch/yaw, and target selection heuristics. |
| **AIM (Touch)** | Drives aim by **synthesizing native touch swipes** instead of writing rotation memory. | `PTFakeTouch` + IOKit `HID` event injection; a closed-loop controller that mimics a real finger. |
| **Loot / Items ESP** | Lists ground loot and items with names resolved from the engine's name table. | `FName` resolution and an item id → display-name table. |

---

## Getting Started

```bash
git clone https://github.com/MengXi47/anticheat-research
cd anticheat-research
python3 init.py
```

## License

This repository is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details. By downloading, cloning, or interacting with this repository, you agree to use its contents solely for authorized security research and educational exploration.
