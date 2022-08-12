// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

using Smdn.Xml.Xhtml;

class DemoServer {
  private const string ServerName = "PolishNotationDemoServer/1.0";
  private const int DefaultLocalPortNumber = 48080;
  private static readonly Encoding utf8nobom = new UTF8Encoding(false);

  static async Task<int> Main(string[] args)
  {
    if (!HttpListener.IsSupported) {
      Console.Error.WriteLine($"{nameof(HttpListener)} is not supported on this platform");
      return 1;
    }

    var localPortNumber = DefaultLocalPortNumber;

    for (var index = 0; index < args.Length; index++) {
      switch (args[index]) {
        case "--port":
          localPortNumber = int.Parse(args[++index]);
          break;
      }
    }

    using var listener = new HttpListener();

    listener.Prefixes.Add($"http://localhost:{localPortNumber}/");

    Console.WriteLine($"server {ServerName} starting (prefix: {string.Join(", ", listener.Prefixes)}) ...");

    listener.Start();

    if (listener.IsListening) {
      Console.WriteLine($"server {ServerName} running");
    }
    else {
      Console.Error.WriteLine("server not listening (something wrong?)");
      return 1;
    }

    try {
      for (; ; ) {
        await ProcessRequestAsync(await listener.GetContextAsync());
      }
    }
    finally {
      listener.Stop();
    }
  }

  private static async Task ProcessRequestAsync(HttpListenerContext context)
  {
    try {
      try {
        var request = context.Request;

        Console.Write(
          "{0} - {1} [{2:r}] \"{3} {4} HTTP/{5}\" {6} ",
          request.RemoteEndPoint,
          request.UserHostName,
          DateTimeOffset.Now,
          request.HttpMethod,
          request.RawUrl,
          request.ProtocolVersion,
          request.ContentLength64
        );

        var (statusCode, responseContent) = await ProcessRequestAsyncCore(context);

        context.Response.StatusCode = (int)statusCode;

        var response = context.Response;

        response.Headers[HttpResponseHeader.Server] = ServerName;
        response.ContentEncoding = utf8nobom;
        response.KeepAlive = false;

        using var s = new MemoryStream();
        using (var writer = new StreamWriter(s, response.ContentEncoding, 1024, true)) {
          if (responseContent is null || responseContent.Content is null) {
            // set default content body
            response.ContentType = "text/plain";

            writer.WriteLine($"{response.StatusCode:D3} {(HttpStatusCode)response.StatusCode}");
          }
          else {
            response.ContentType = responseContent.ContentType;

            writer.WriteLine(responseContent.Content);
          }

          writer.Flush();
        }

        response.ContentLength64 = s.Length;

        s.Position = 0L;

        Console.WriteLine(
          "{0:D3} {1} \"{2}\" \"{3}\"",
          response.StatusCode,
          s.Length,
          request.UrlReferrer,
          request.UserAgent
        );

        s.CopyTo(response.OutputStream);
      }
      catch {
        throw;
      }
    }
    finally {
      if (context != null)
        context.Response.OutputStream.Close();
    }
  }

  private class ResponseContent {
    public string? Content { get; }
    public string ContentType { get; }

    public ResponseContent(string content, string contentType)
    {
      Content = content;
      ContentType = contentType;
    }
  }

  private const string ContentTypeCascadingStyleSheets = "text/css; charset=UTF-8";
  private const string ContentTypeJavaScript = "text/javascript; charset=UTF-8";

  private static readonly IReadOnlyDictionary<string, Func<Task<ResponseContent>>> contentLoader =
    new Dictionary<string, Func<Task<ResponseContent>>>(StringComparer.Ordinal) {
      {
        "/",
        static async () => new(
          content: await ConstructIndexAsync(),
          contentType: "text/html; charset=UTF-8"
        )
      },
      {
        "/polish.js",
        static async () => new(
          content: await File.ReadAllTextAsync(Path.Join(Paths.ImplementationsBasePath, "javascript/polish.js")),
          contentType: ContentTypeJavaScript
        )
      },
      {
        "/polish-demo.css",
        static async () => new(
          content: await File.ReadAllTextAsync(Path.Join(Paths.ContentsBasePath, "polish-demo.css")),
          contentType: ContentTypeCascadingStyleSheets
        )
      },
      {
        "/polish-demo.js",
        static async () => new(
          content: await File.ReadAllTextAsync(Path.Join(Paths.ContentsBasePath, "polish-demo.js")),
          contentType: ContentTypeJavaScript
        )
      },
      {
        "/polish-expressiontree.css",
        static async () => new(
          content: await File.ReadAllTextAsync(Path.Join(Paths.ContentsBasePath, "polish-expressiontree.css")),
          contentType: ContentTypeCascadingStyleSheets
        )
      },
      {
        "/polish-expressiontree.js",
        static async () => new(
          content: await File.ReadAllTextAsync(Path.Join(Paths.ContentsBasePath, "polish-expressiontree.js")),
          contentType: ContentTypeJavaScript
        )
      },
    };

  private static async Task<(HttpStatusCode, ResponseContent?)> ProcessRequestAsyncCore(
    HttpListenerContext context
  )
  {
    // can be accessed from local address
    if (!IPAddress.IsLoopback(context.Request.RemoteEndPoint.Address))
      return (HttpStatusCode.Forbidden, null);

    try {
      if (context.Request.RawUrl is null) {
        return (
          HttpStatusCode.BadRequest,
          null
        );
      }

      if (contentLoader.TryGetValue(context.Request.RawUrl, out var loadAsync)) {
        return (
          HttpStatusCode.OK,
          await loadAsync()
        );
      }

      return (
        HttpStatusCode.NotFound,
        null
      );
    }
    catch (Exception ex) {
      Console.Error.WriteLine(ex);

      return (
        HttpStatusCode.InternalServerError,
        new ResponseContent(
          content: ex.ToString(),
          contentType: "text/plain; charset=UTF-8"
        )
      );
    }
  }

  private static async Task<string> ConstructIndexAsync()
  {
    XDocument templateIndex;
    XDocument fragmentPolishDemo;

    using (var stream = File.OpenRead(Path.Join(Paths.ContentsBasePath, "index.template.xhtml"))) {
      templateIndex = await XDocument.LoadAsync(stream, LoadOptions.None, cancellationToken: default);
    }

    using (var stream = File.OpenRead(Path.Join(Paths.ContentsBasePath, "polish-demo.fragment.xhtml"))) {
      var nsmgr = new XmlNamespaceManager(new NameTable());

      nsmgr.AddNamespace(string.Empty, "http://www.w3.org/1999/xhtml"); // set default xml namespace to XHTML's one

      var context = new XmlParserContext(null, nsmgr, null, XmlSpace.Default);
      var settings = new XmlReaderSettings() {
        Async = true,
        DtdProcessing = DtdProcessing.Ignore,
        CloseInput = true,
        ConformanceLevel = ConformanceLevel.Fragment,
        IgnoreComments = true,
        IgnoreWhitespace = true,
      };

      var reader = XmlReader.Create(stream, settings, context);

      fragmentPolishDemo = await XDocument.LoadAsync(reader, LoadOptions.None, cancellationToken: default);
    }

    var nsPlaceholder = (XNamespace)"https://github.com/smdn/polish-notation-impls";
    var elementPlaceholder = templateIndex.Descendants(nsPlaceholder + "placeholder").FirstOrDefault();

    if (elementPlaceholder is null)
      throw new InvalidOperationException("placeholder element not found");

    elementPlaceholder.AddAfterSelf(fragmentPolishDemo.Root);
    elementPlaceholder.Remove();

    var sb = new StringBuilder(10 * 1024);
    var writerSettings = new XmlWriterSettings() {
      Async = false, // async is not implemented
      CloseOutput = true,
      Indent = true,
      IndentChars = " ",
      NewLineChars = "\n",
    };

    using (var writer = new PolyglotHtml5Writer(sb, writerSettings)) {
      // await templateIndex.SaveAsync(writer, cancellationToken: default); // async is not implemented
      templateIndex.Save(writer);
    }

    return sb.ToString();
  }
}
