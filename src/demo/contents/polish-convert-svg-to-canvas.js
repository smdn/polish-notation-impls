// SPDX-FileCopyrightText: 2022 smdn <smdn@smdn.jp>
// SPDX-License-Identifier: MIT

"use strict";

let getComputedCSSText = node =>
{
  let style = window.getComputedStyle(node, "");

  if (!style.cssText || style.cssText === '') {
    let cssText = "";

    for (let i = 0; i < style.length; i++) {
      cssText += style[i] + ': ' + style.getPropertyValue(style[i]) + '; ';
    }

    return cssText.trim();
  }
  else {
    return style.cssText;
  }
};

function setSvgElementBeginTime(svg, beginOffset)
{
  // alternate SVGSVGElement.setCurrentTime
  Array.prototype.forEach.call(
    svg.querySelectorAll("animate, animateMotion"),
    (e) =>
    {
      let begin = e.getAttribute("begin");

      begin = Number.parseInt(begin.replace(/s$/g, ''));
      begin += beginOffset;

      e.setAttribute("begin", begin + "s");
    }
  );
}

function getSvgElementDuration(svg)
{
  let durMax = 0;

  Array.prototype.forEach.call(
    svg.querySelectorAll("animate, animateMotion"),
    (e) =>
    {
      let begin = 0; // Number.parseInt(e.getAttribute("begin"));
      let dur = Number.parseInt(e.getAttribute("dur"));

      durMax = Math.max(durMax, begin + dur);
    }
  );

  return durMax;
}

function formatTime(number)
{
  let nf = new Intl.NumberFormat(
    "en",
    {
      style: "decimal",
      minimumIntegerDigits: 2,
      minimumFractionDigits: 2,
      maximumFractionDigits: 2,
      useGrouping: false,
    }
  );

  return nf.format(number);
}

function emitCanvasFramesFromSvg(svg, svgDefs, autoDownload)
{
  let w = svg.width.baseVal.value;
  let h = svg.height.baseVal.value;

  if (!(0 < w && 0 < h))
    return;

  let duration = getSvgElementDuration(svg);
  let filenameTrunk = svg.parentNode.dataset.filename.replace(/\..+$/g, "");
  const interval = 0.25; // in [secs]

  for (let time = 0; time < duration; time += interval) {
    let filename = filenameTrunk + "-" + formatTime(time) + "s";

    //let canvas = svg.parentNode.insertBefore(document.createElement("canvas"), svg.nextSibling);
    let canvas = svg.parentNode.appendChild(document.createElement("canvas"));

    canvas.width = w;
    canvas.height = h;
    canvas.title = filename;

    let ctx = canvas.getContext("2d");
    let image = new Image;

    image.onload = () =>
    {
      ctx.fillStyle = "white";
      ctx.fillRect(0, 0, w, h);
      ctx.drawImage(image, 0, 0);

      if (autoDownload) {
        let a = document.body.appendChild(document.createElement("a"));

        a.href = canvas.toDataURL("image/png");
        a.setAttribute("download", filename + ".png");
        a.click();
        //a.dispatchEvent(new Event("click"));
      }
    };

    let s = svg.cloneNode(true);

    s.appendChild(svgDefs.cloneNode(true));

    //s.style.cssText = getComputedCSSText(svg); // not works
    Array.prototype.forEach.call(
      document.querySelectorAll("style"),
      (style) =>
      {
        s.appendChild(style.cloneNode(true));
      }
    );

    setSvgElementBeginTime(s, -time);

    let svgData = new XMLSerializer().serializeToString(s);

    image.src = "data:image/svg+xml;charset=utf-8;base64," + btoa(unescape(encodeURIComponent(svgData)));
  }
}
