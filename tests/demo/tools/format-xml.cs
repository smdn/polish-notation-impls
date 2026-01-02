#!/usr/bin/env dotnet
//
// requirements:
//   .NET SDK 10 or over
//   (see https://learn.microsoft.com/dotnet/core/sdk/file-based-apps)
//
// usage:
//   ./format-xml.csx a.xml b.xml
//

using System.Xml;
using System.Xml.Linq;
using System.Text;

var settings = new XmlWriterSettings() {
  Encoding = new UTF8Encoding(false),
  Indent = true,
  IndentChars = " ",
  NewLineChars = "\n",
  NewLineOnAttributes = true
};

foreach (var file in args) {
  Console.Write($"reformatting {file} ... ");

  var doc = XDocument.Load(file);

  using (var writer = XmlWriter.Create(file, settings)) {
    doc.Save(writer);
  }

  Console.WriteLine("done");
}
