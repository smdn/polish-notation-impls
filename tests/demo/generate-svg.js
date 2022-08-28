#!/usr/bin/env node

import * as fs from "fs";
import { execSync } from "child_process";
import { JSDOM } from 'jsdom';
import {
  Svg,
  ExpressionTreeNode,
  VisualTreeNode,
  TraversalOrderRenderer,
  CalculationTransitionRenderer
} from '../../src/demo/contents/polish-expressiontree.mjs'

function main(expression) {
  let root = new ExpressionTreeNode(expression);

  root.parseExpression();

  let visualizations = [
    {
      elementId: "polish-demo-expressiontree-traverse-preorder",
      outputFileName: "expressiontree-traverse-preorder.actual.svg",
      action: (tree, target) => {
        tree.render(target, true);
        tree.traversePathPreorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      elementId: "polish-demo-expressiontree-traverse-postorder",
      outputFileName: "expressiontree-traverse-postorder.actual.svg",
      action: (tree, target) => {
        tree.render(target, true);
        tree.traversePathPostorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      elementId: "polish-demo-expressiontree-traverse-inorder",
      outputFileName: "expressiontree-traverse-inorder.actual.svg",
      action: (tree, target) => {
        tree.render(target, true);
        tree.traversePathInorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      elementId: "polish-demo-expressiontree-calculation",
      outputFileName: "expressiontree-calculation.actual.svg",
      action: (tree, target) => {
        tree.calculate_expression_tree(new CalculationTransitionRenderer(target));
      },
    },
  ];

  visualizations.forEach(v => {
    let target = VisualTreeNode.initializeSvgElement(
      VisualTreeNode.createSvgElement(v.elementId)
    );

    v.action(root.createVisualTree(), target);

    const maxWidth = 1080;

    Svg.minimizeElement(target, 5, maxWidth);

    fs.writeFileSync(
      v.outputFileName,
      target.outerHTML,
      {
        flags: "w",
        encoding: "utf8"
      },
      (err) => { if (err) throw err; }
    );

    execSync(`./tools/format-xml.csx ${v.outputFileName}`);

    process.stdout.write(`generated ${v.outputFileName}\n`);
  });

  process.exit(0);
}

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

const expression = "2 + 5 * 3 - 4";

main(expression);
