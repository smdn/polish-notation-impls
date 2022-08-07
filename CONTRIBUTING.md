# Contribution guidelines
本リポジトリではコードやドキュメントの修正・改善、不具合の報告などの貢献を歓迎します。

IssueやPull Requestを作成する際は、本文をご一読ください。

# リポジトリの目的
本リポジトリに含まれる実装は、[二分木を使った数式の逆ポーランド記法化と計算](https://smdn.jp/programming/tips/polish/)で解説している数式の二分木化・記法変換・計算を実装することに主眼をおいています。

したがって、記法変換および数式計算に関する**機能の高度化や性能の追求は目的としていません**。　この目的から逸脱するような変更は受け入れられない場合があるのでご了承ください。

# Issue / 不具合の報告
ある程度テストをした状態で公開していますが、未発見の不具合があるかもしれません。

不具合を発見した場合は、以下の情報を添えて[Issue](https://github.com/smdn/polish-notation-impls/issues/)にてご報告ください。
- 入力に与えた内容と、期待される出力、実際の出力
- エラーとなった場合は、そのエラーメッセージ
- 不具合が起こった実装の言語
- 実行環境 (OS情報・ランタイムバージョンなど)

なお、各実装がサポートする機能と、制限事項については[二分木を使った数式の逆ポーランド記法化と計算 §Cでの実装 機能と制限](https://smdn.jp/programming/tips/polish/#Implementation_C_FeaturesAndLimitations)を参照してください。

不具合以外にも、改善点等の提案がある場合なども[Issue](https://github.com/smdn/polish-notation-impls/issues/)にてお知らせください。

# Pull Request / コードの変更
不具合の修正や、コードに改善点や変更したい箇所がある場合は、[Pull Request](https://github.com/smdn/polish-notation-impls/pulls/)を作成してください。

## Pull Requests are welcome
主に以下のような内容でのPull Requestを歓迎します。　この他の変更が可能かどうかについて疑問がある場合は、まず[Issue](https://github.com/smdn/polish-notation-impls/issues/)にてお問い合わせください。

This repository uses mainly Japanese, pull requests in English are also welcome.

### 最新の記法・構文へのキャッチアップ
言語の最新**安定版**でサポートされる記法・構文・APIにキャッチアップする変更を歓迎します。

実装によっては、現在では非推奨となっている関数・APIを使用している可能性があるので、それをリプレースするような変更も歓迎します。

### 他言語への移植
他言語版の作成(他言語での実装の追加)を歓迎します。

ただし、他言語版を作成する場合は[C言語](/src/impls/c/)もしくは[C#](/src/impls/csharp/)の実装をベースに作成してください。　その際、[解説原文](https://smdn.jp/programming/tips/polish/)で掲載しているC言語のコードを、**新たに作成する他言語版に置き換えても原文はそのまま読めるように**、以下の要綱に従ってください。

- 関数・条件分岐・処理・コメント文が、ベースの実装と可能な限り1対1で対応するような形で記述してください。
- 関数と処理の順序は、言語による制限がない限り、ベースの実装と合わせてください。
- 関数や変数の命名、コメント文の内容は、変更しないでください。
  - ただし命名について、pascal-case/camel-case/snake-case/kebab-caseなどのcasingに関しては、その言語での標準・慣習に従うように変更してかまいません。
- shebang・ディレクティブ・初期化処理や標準入出力等、その言語に特有・必須な部分については、そのまま記述してください。

## Unacceptable changes / 受け入れられない変更
[リポジトリの目的](#リポジトリの目的)から逸脱するような変更は受け入れられません。　例として、以下のような変更はお断りします。

- 速度や効率など、パフォーマンスのみを追求するような変更
- ショートコーディングを追求するような変更
- サードパーティーパッケージの使用など、標準ライブラリ以外への依存や、依存関係を複雑化させるような変更
- 機能の追加、または既存機能の削除・変更など、[解説原文](https://smdn.jp/programming/tips/polish/)側に影響が及ぶ変更
  - 機能の追加したい場合は、解説原文で掲載するためのアルゴリズムの解説も同時に書いていただけるなら、受け入れ可能です。　コード変更のみでの機能追加は受け入れられません。

## コード変更に関する注意事項

### コードフォーマット
コードを変更する際、インデントやスペースなどのコードフォーマットは、既存のコードおよび`.editorconfig`ファイルで定義されているフォーマットに(できる限りで構わないので)、従うようにしてください。　コードフォーマットが逸脱していてもかまいませんが、Pull Requestのマージ前後で修正する場合があります。

### テスト
コードを変更する際は、(可能であれば)変更箇所をカバーするテストケース・テストコードをご提示ください。

### 著作権表示
コードに変更を加える際に、希望する場合は著作権表示を追加することもできます。　その場合、以下のようにソースファイルのヘッダ部分に[SPDX short-form identifiers](https://spdx.dev/ids/)を追記してください。

```diff
 // SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
+// SPDX-FileCopyrightText: 20XX your-name <your-contact-address@example.jp>
 // SPDX-License-Identifier: MIT
```

ただし、**本人以外の著作権表示およびコードライセンスを削除・変更するような修正は受け入れられません**。

### マージ処理
Pull Requestをマージする場合、基本的には[Create a merge commit](https://docs.github.com/ja/pull-requests/collaborating-with-pull-requests/incorporating-changes-from-a-pull-request/about-pull-request-merges)でマージします。　希望する場合は[Squash and merge](https://docs.github.com/ja/pull-requests/collaborating-with-pull-requests/incorporating-changes-from-a-pull-request/about-pull-request-merges#squash-and-merge-your-pull-request-commits)でマージしますのでお申し出ください。

