#!/usr/bin/env dotnet-script
//
// requirements:
//   .NET Runtime
//
// usage:
//   1. install dotnet-script
//     dotnet tool install -g dotnet-script
//   2. run this script
//     ./format-xml.csx a.xml b.xml
//     dotnet script ./format-xml.csx a.xml b.xml

using System;
using System.Xml;
using System.Xml.Linq;
using System.Text;

var settings = new XmlWriterSettings() {
  Encoding = new UTF8Encoding(false),
  Indent = true,
  IndentChars = " ",
  NewLineChars = "\n",
};

foreach (var file in Args) {
  Console.Write($"reformatting {file} ... ");

  var doc = XDocument.Load(file);

  using (var writer = XmlWriter.Create(file, settings)) {
    doc.Save(writer);
  }

  Console.WriteLine("done");
}
