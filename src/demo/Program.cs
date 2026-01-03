// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
using System;
using System.CommandLine;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;

using Microsoft.Extensions.DependencyInjection;

using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Logging.Console;

using Smdn.Xml.Xhtml;

const int DefaultLocalPortNumber = 48080;

const string ContentTypeCascadingStyleSheets = "text/css; charset=UTF-8";
const string ContentTypeJavaScript = "text/javascript; charset=UTF-8";

var applicationName = Assembly.GetEntryAssembly()?.FullName ?? "DemoServer";

var builder = WebApplication.CreateBuilder(
  options: new() {
    Args = args,
    ApplicationName = applicationName,
  }
);

builder.Logging.AddSimpleConsole(static options => {
  options.SingleLine = true;
  options.TimestampFormat = "MM-ddTHH:mm:ss ";
  options.UseUtcTimestamp = false;
  options.ColorBehavior = LoggerColorBehavior.Enabled;
});

var localPortNumberOption = new Option<int>("--port") {
  Description = "Port number the demo server listens on.",
  DefaultValueFactory = static _ => DefaultLocalPortNumber,
};

var rootCommand = new RootCommand(applicationName);

rootCommand.Options.Add(localPortNumberOption);

rootCommand.SetAction(
  action: async (parseResult, cancellationToken) => {
    var localPortNumber = parseResult.GetValue(localPortNumberOption);

    builder.WebHost.ConfigureKestrel(
      (context, serverOptions) => {
        serverOptions.AddServerHeader = false;
        serverOptions.AllowSynchronousIO = true;

        serverOptions.ListenLocalhost(localPortNumber);
      }
    );

    using var app = builder.Build();

    app.MapGet(
      pattern: "/",
      handler: async (HttpContext context, CancellationToken cancellationToken) => {
        context.Response.ContentType = "text/html; charset=utf-8";
        context.Response.StatusCode = 200;

        await ConstructIndexAsync(
          context.Response.Body,
          cancellationToken
        ).ConfigureAwait(false);
      }
    );

    app.MapGet(
      pattern: "/{name}.css",
      handler: (string name) => name switch {
        "polish-demo" or
        "polish-expressiontree"
          => Results.File(Path.Join(Paths.ContentsBasePath, $"{name}.css"), ContentTypeCascadingStyleSheets),

        _ => Results.NotFound(),
      }
    );

    app.MapGet(
      pattern: "/{name}.mjs",
      handler: (string name) => name switch {
        "Node" or
        "polish-demo" or
        "polish-expressiontree"
          => Results.File(Path.Join(Paths.ContentsBasePath, $"{name}.mjs"), ContentTypeJavaScript),

        _ => Results.NotFound(),
      }
    );

    await app.RunAsync().ConfigureAwait(false);

    return 0;
  }
);

return await rootCommand.Parse(args).InvokeAsync().ConfigureAwait(false);

static async Task ConstructIndexAsync(
  Stream destination,
  CancellationToken cancellationToken
)
{
  XDocument templateIndex;

  using (var stream = File.OpenRead(Path.Join(Paths.ContentsBasePath, "index.template.xhtml"))) {
    templateIndex = await XDocument.LoadAsync(
      stream,
      LoadOptions.None,
      cancellationToken
    ).ConfigureAwait(false);
  }

  var nsPlaceholder = (XNamespace)"https://github.com/smdn/polish-notation-impls";

  foreach (var elementPlaceholder in templateIndex.Descendants(nsPlaceholder + "placeholder").ToList()) {
    var file = elementPlaceholder.Attribute("file")?.Value;

    using (var stream = File.OpenRead(Path.Join(Paths.ContentsBasePath, file))) {
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

      var replacement = await XDocument.LoadAsync(
        reader,
        LoadOptions.None,
        cancellationToken
      ).ConfigureAwait(false);

      elementPlaceholder.AddAfterSelf(replacement.Root);
      elementPlaceholder.Remove();
    }
  }

  var writerSettings = new XmlWriterSettings() {
    Async = true,
    CloseOutput = true,
    Indent = true,
    IndentChars = " ",
    NewLineChars = "\n",
  };

  var writer = new PolyglotHtml5Writer(destination, writerSettings);

  await using (writer.ConfigureAwait(false)) {
    await templateIndex.SaveAsync(
      writer,
      cancellationToken
    ).ConfigureAwait(false);
  }
}
