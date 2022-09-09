# 各実装のテスト
## 前提条件
テストランナーの実行には[PowerShell 7.0](https://docs.microsoft.com/en-us/powershell/scripting/whats-new/what-s-new-in-powershell-70)以降が必要です。

## テストの実行方法
スクリプト[run-tests.ps1](./run-tests.ps1)を実行すると、[implementations.jsonc](./implementations.jsonc)に記述されているすべての実装に対してテストを行います。

```sh
./run-tests.ps1
```

オプション`-target-impl <implementation-id>`を指定して実行すると、指定されたIDに対応する実装のみに対してテストを行います。

```pwsh
./run-tests.ps1 -target-impl javascript
```

bash等で引数を指定する場合は、オプション名の前にハイフンを2つ前置してください。

```sh
./run-tests.ps1 --target-impl javascript
```

オプション`-v`を指定して実行すると冗長モードとなり、各テストケースの実行結果が表示されます。

```pwsh
./run-tests.ps1 -v
```

## テストの編集
### テストケース
テストケースは[testcases](./testcases/)ディレクトリの各`.jsonc`ファイルに記述されています。

テストケースは、入力となる式を`Input`で与え、それに対して期待される動作と結果(`Expected***`, `ExpectAs***`)を記述します。

### テスト対象の実装
実装の一覧は[implementations.jsonc](./implementations.jsonc)に記述されています。

実装を追加する場合や、異なるコンパイラを使用したテストを行う場合は、テスト対象の実装があるディレクトリと、ビルド・クリーン・実行を行うための各コマンドを記述してください。

このディレクトリにあるテストスクリプトは、GitHub Actionsのワークフロー[run-tests-impls.yml](/.github/workflows/run-tests-impls.yml)から呼び出されます。　GitHub Actionsでテストする場合は、左記ワークフローにてテストの実行に必要な依存関係をインストールするよう設定する必要があります。
