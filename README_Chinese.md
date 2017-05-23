# hast(1.0.0)

在Linux平台上的socket溝通套件，並針對[網路拓樸](https://zh.wikipedia.org/wiki/%E7%BD%91%E7%BB%9C%E6%8B%93%E6%89%91)特別設計，支援TCP/IP和unix domain socket，特色是平行處理和容易使用

## 介紹影片

* 抽象層
  - [English](https://www.youtube.com/watch?v=EpoL8mSOA6E)(版本 < 1.0.0, 過期)
  - [中文](https://www.youtube.com/watch?v=G41F7xHC2bs)(版本 < 1.0.0, 過期)
* API Layer (WIP)
* Code Layer (WIP)

## 伺服器端套件特色

* 類似GO語言中的`goroutine`，使用者設定最多有多少執行緒來處理請求，但執行敘數目並不是傳統的thread pool設計，執行緒數量是動態調整的，數量會介於1~最大數目之間，按照忙碌程度增減。
* 伺服器端可以被「呼叫暫停」，可以是全部執行緒都暫停，或是特定請求的執行緒暫停，更多詳細訊息請參閱`example`資料夾。

## 客戶端套件特色

* `client_core`是同步的客戶端套件，沒有多執行緒的設計，單純傳送訊息並等著接收。
* `client_thread`是多執行緒的客戶端套件，傳送訊息後會有其他執行緒去接收訊息，不會阻礙到主執行緒。

## 開始使用

* 只適用於Linux平台，kernel > 2.5.44（因為使用到epoll.h）
* 需要一個可支援C++11的編譯器
* 沒有其他依附套件，只需把`hast`資料夾複製到系統的`include`資料夾裡即可
* 使用標準函式庫中的`std::thread`, 所以編譯時需要使用到該套件（可能是`-pthread`）

## Wiki資料

* 更多詳細資訊請參閱此專案的[wiki](https://github.com/hn12404988/hast/wiki).

## 框架

* 如果你在建立一個Linux伺服器，建議可以參考另一個我的專案叫[dalahast](https://github.com/hn12404988/dalahast)，這是一個完整從Web前端到hast後端的示範框架。
* [hast_web](https://github.com/hn12404988/hast_web)是一個基於這個專案的另一個發展方向，移除了拖墣的功能，並強化平行處理的能力，可以對單一socket接收多次訊息並平行處理，但只支援[WebSocket](https://zh.wikipedia.org/wiki/WebSocket)端口。

## 缺陷和建議

這專案仍在開發中，所以有發現任何缺陷或建議，歡迎使用issue功能回報。
