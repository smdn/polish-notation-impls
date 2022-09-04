#!/usr/bin/env node

import * as fs from "fs";
import { JSDOM } from 'jsdom';
import {
  Svg,
  ExpressionTreeNode,
  VisualTreeNode,
  TraversalOrderRenderer,
  CalculationTransitionRenderer
} from '../../src/demo/contents/polish-expressiontree.mjs'

function generate(
  expression,
  visualizationMode,
  elementId,
  outputPath
) {
  if (!expression || expression.replaceAll(" ", "").length == 0) {
    process.stderr.write("invalid expression\n");
    process.exit(1);
  }

  let root = new ExpressionTreeNode(expression.replaceAll(" ", ""));

  root.parseExpression();

  let visualizationAction = function(mode) {
    switch (mode) {
      case "traverse-preorder":
        return (tree, target) => {
          tree.render(target, true);
          tree.traversePathPreorder(new TraversalOrderRenderer(target, null));
        };
      case "traverse-postorder":
        return (tree, target) => {
          tree.render(target, true);
          tree.traversePathPostorder(new TraversalOrderRenderer(target, null));
        };
      case "traverse-inorder":
        return (tree, target) => {
          tree.render(target, true);
          tree.traversePathInorder(new TraversalOrderRenderer(target, null));
        };
      case "calculate":
        return (tree, target) => {
          tree.calculate_expression_tree(new CalculationTransitionRenderer(target));
        };

      default:
        throw `undefined visualization mode: ${mode}`
    }
  }(visualizationMode);

  let target = VisualTreeNode.initializeSvgElement(
    VisualTreeNode.createSvgElement(elementId)
  );

  visualizationAction(root.createVisualTree(), target);

  const maxWidth = 1080;

  Svg.minimizeElement(target, 5, maxWidth);

  fs.writeFileSync(
    outputPath,
    target.outerHTML,
    {
      flags: "w",
      encoding: "utf8"
    },
    (err) => { if (err) throw err; }
  );

  process.stdout.write(`generated ${outputPath}\n`);
  process.exit(0);
}

function main()
{
  // TODO: generate SVG with headless Firefox + Selenium
  const pseudoDOM = new JSDOM('');

  // expose `window` and `document` to the global scope
  global.window = pseudoDOM.window;
  global.document = pseudoDOM.window.document;
  
  // define SVGElement.getBBox
  // ref: https://github.com/jsdom/jsdom/issues/3159
  Object.defineProperty(window.SVGElement.prototype, 'getBBox', {
    writable: true,
    value: () => ({
      x: 0,
      y: 0,
      width: 0,
      height: 0
    })
  });

  var expression;
  var visualizationMode;
  var elementId;
  var outputPath;

  for (var i = 0; i < process.argv.length; i++) {
    switch (process.argv[i]) {
      case "--input-expression":
        expression = process.argv[++i];
        break;

      case "--visualization-mode":
        visualizationMode = process.argv[++i];
        break;

      case "--output-element-id":
        elementId = process.argv[++i];
        break;

      case "--output-path":
        outputPath = process.argv[++i];
        break;
    }
  }

  generate(
    expression,
    visualizationMode,
    elementId,
    outputPath
  );
}

main();
