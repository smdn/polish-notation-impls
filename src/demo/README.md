# 記法変換と数式計算のデモ
このデモでは、[JavaScriptの実装](../impls/javascript/)をブラウザ上で実行することができます。　また、各記法への変換過程および数式の変換過程をアニメーションで確認することができます。

## 実行方法
まずはデモ用のHTTPサーバーを起動します。　以下のコマンドで起動してください。

```sh
dotnet run # デフォルトのポート番号48080でデモサーバーを起動する
dotnet run -- --port 8080 # ポート番号8080でデモサーバーを起動する
```

デモサーバーは、デフォルトではポート番号48080を使用します。　別のサーバーがこのポートを使用している場合は、引数`--port <port-number>`でポート番号を変更することができます。

デモサーバーが起動したら、ブラウザで[http://localhost:48080/](http://localhost:48080/)を開いてください。

## 制限事項
デモサーバーの実行には、バージョン10.0以上の[.NET SDK](https://docs.microsoft.com/ja-jp/dotnet/core/sdk)、および[ASP.NET Core ランタイム](https://dotnet.microsoft.com/ja-jp/apps/aspnet)を使用できるプラットフォームが必要です。

このデモの実行には、**SVG**(Scalable Vector Graphics)および**SVG SMIL**(Synchronized Multimedia Integration Language)に対応したブラウザが必要です。
