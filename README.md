スライドパズル 解法探索アルゴリズム for Android
====

**Google Developer Day 2011**で参加資格を得るために行われた、DevQuizのスライドパズルの解法を探索するプログラムです。  
完全に同一のアルゴリズムを足回りだけ変更した3種類の実装で実行速度を比べることが出来ます。  
Dalvik/ART/NDKでの速度特性の違いを比較するために開発したアプリです。

詳細な考察は下記の記事にまとめています。  
http://codezine.jp/article/detail/8333  
http://techblog.raccoon.ne.jp/archives/42593044.html

## 動かし方
Android Studioの利用を前提にしています。  
Import projectでルートディレクトリのbuild.gradleを指定するとプロジェクトとして取り込めます。

## 実装について
### Start(VM)
探索アルゴリズムの標準の実装です。

### Start(Arrayed)
頻繁に生成・破棄されるオブジェクトをintの巨大配列に置き換えることで該当オブジェクトの生成・破棄のオーバーヘッドをゼロにした実装です。  
Dalvikでは標準の実装に比べて2倍以上高速になります。

### Start(Native)
探索アルゴリズムのC言語での実装です。  
Dalvikで動かした標準の実装に比べて4倍以上も高速化します。
