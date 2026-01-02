// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
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

var builder = WebApplication.CreateBuilder(
  options: new() {
    Args = args,
    ApplicationName = Assembly.GetEntryAssembly()?.FullName ?? "DemoServer",
  }
);

var localPortNumber = DefaultLocalPortNumber;

for (var index = 0; index < args.Length; index++) {
  switch (args[index]) {
    case "--port":
      localPortNumber = int.Parse(args[++index]);
      break;
  }
}

builder.WebHost.ConfigureKestrel(
  (context, serverOptions) => {
    serverOptions.AddServerHeader = false;

    serverOptions.ListenLocalhost(localPortNumber);
  }
);

builder.Logging.AddSimpleConsole(static options => {
  options.SingleLine = true;
  options.TimestampFormat = "MM-ddTHH:mm:ss ";
  options.UseUtcTimestamp = false;
  options.ColorBehavior = LoggerColorBehavior.Enabled;
});

using var app = builder.Build();

app.MapGet(
  pattern: "/",
  handler: async (CancellationToken cancellationToken)
    => Results.Content(
      content: await ConstructIndexAsync(cancellationToken).ConfigureAwait(false),
      contentType: "text/html; charset=utf-8"
    )
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

static async Task<string> ConstructIndexAsync(CancellationToken cancellationToken)
{
  XDocument templateIndex;

  using (var stream = File.OpenRead(Path.Join(Paths.ContentsBasePath, "index.template.xhtml"))) {
    templateIndex = await XDocument.LoadAsync(stream, LoadOptions.None, cancellationToken);
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

      var replacement = await XDocument.LoadAsync(reader, LoadOptions.None, cancellationToken);

      elementPlaceholder.AddAfterSelf(replacement.Root);
      elementPlaceholder.Remove();
    }
  }

  var sb = new StringBuilder(10 * 1024);
  var writerSettings = new XmlWriterSettings() {
    Async = false, // async is not implemented
    CloseOutput = true,
    Indent = true,
    IndentChars = " ",
    NewLineChars = "\n",
  };

  using (var writer = new PolyglotHtml5Writer(sb, writerSettings)) {
    // await templateIndex.SaveAsync(writer, cancellationToken); // async is not implemented
    templateIndex.Save(writer);
  }

  return sb.ToString();
}
