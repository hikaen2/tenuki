# tenukiについて
tenuki（手抜き）は簡単なコンピュータ将棋エンジンです。
CSAプロトコル (http://www.computer-shogi.org/protocol/) を話します。

## ソース
https://github.com/hikaen2/tenuki

## ライセンス
MIT License

## 開発の経緯
Bonanzaのコードが難しいと感じたため，簡単な将棋を作りたくなりました。

## 特長
- C++11
- オブジェクティブでないコード
- データ構造はマス目の2次元配列
- テンプレートを書かない
- マクロを書かない
- 機械学習をしない
- DRYなコード

## 評価関数
駒得のみ。

## 探索
αβ法で全幅探索します。
