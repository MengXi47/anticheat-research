# UE4 行動遊戲進程內疊圖 — 反作弊研究樣本（僅供教育與研究用途）

![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)
![Category](https://img.shields.io/badge/Category-Security%20Research-red.svg)
![Platform](https://img.shields.io/badge/Platform-iOS%20(arm64)-green.svg)
![Engine](https://img.shields.io/badge/Target-Unreal%20Engine%204-purple.svg)

> [English](README.md) ｜ 繁體中文

本儲存庫提供一份概念性樣本，示範在 iOS 上對 Unreal Engine 4 行動遊戲進行進程內函式掛鉤（function hooking）與疊圖渲染（overlay rendering）的實作技術。本專案僅供教育、反作弊研究與逆向工程學習之用。

## 重要免責聲明

**關鍵：本專案僅供教育與研究用途。**

* 本軟體的存在是為了協助安全研究人員理解進程內外掛如何掛鉤遊戲引擎、攔截遊戲資料並繪製疊圖——進而能針對這些手法設計**反作弊對策**。
* 請勿將此程式碼用於正式、公開或生產環境。
* 任何為了在配對中取得不公平優勢而部署的行為，皆違反該遊戲的服務條款，並可能在當地司法管轄區構成違法。
* 作者對於因使用或修改本原始碼而導致的濫用、帳號封禁、法律後果或損害，概不承擔任何責任。

---

## 目標與投遞方式

* **平台：** iOS（arm64），需越獄裝置。
* **目標進程：** 一款基於 Unreal Engine 4 的行動大逃殺遊戲。
* **注入途徑：** payload 被編譯為動態函式庫，並**偽裝成 `libwebp` framework**，使其於宿主進程啟動時被載入。此偽裝必須重新匯出（re-export）真正的 WebP 符號，否則宿主會 crash。
* **渲染：** 同進程疊圖，使用 Dear ImGui 繪製於宿主的 **Metal** 圖層之上（無獨立視窗、非外部讀取進程模型）。

---

## 功能總覽

| 功能 | 技術說明 | 研究重點 |
| :--- | :--- | :--- |
| **過錄影（Hide Record）** | 讓疊圖不被螢幕錄影、截圖與直播串流捕捉，但在裝置本機上仍可見。 | 利用 iOS `UITextField.secureTextEntry`：其底層圖層（backing layer）會被系統排除於畫面捕捉之外，因此將疊圖重新掛載（re-parent）進該安全圖層（`applyHideRecord`）。 |
| **ESP（透視 / 超感知覺）** | 在玩家身上繪製方框、骨架與距離／名稱文字。 | UE4 實體清單走訪、骨骼矩陣擷取，以及 3D→2D 的 World-to-Screen 投影運算。 |
| **AIM（角度自瞄）** | 透過寫入旋轉向量，將鏡頭／準心調整至最近的有效目標。 | 指標鏈（pointer chain）、pitch／yaw 的向量運算，以及目標選取啟發法。 |
| **AIM（觸控自瞄）** | 以**合成原生觸控滑動**驅動瞄準，而非寫入旋轉記憶體。 | `PTFakeTouch` + IOKit `HID` 事件注入；一套模擬真實手指的閉環控制器。 |
| **Loot／物資透視** | 列出地面掉落物與物資，名稱由引擎的名稱表解析而得。 | `FName` 解析，以及物品 ID → 顯示名稱的對照表。 |

---

## 使用方式

```bash
git clone https://github.com/MengXi47/anticheat-research
cd anticheat-research
python3 init.py
```

## 授權

本儲存庫採用 Apache License 2.0 授權。詳見 [LICENSE](LICENSE) 檔案。下載、複製或與本儲存庫互動，即表示你同意僅將其內容用於經授權的安全研究與教育探索。
