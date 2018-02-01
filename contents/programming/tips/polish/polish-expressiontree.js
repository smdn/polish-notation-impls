"use strict"

class PseudoStdOut {
  constructor()
  {
    this._buffer = [];
  }

  write(val)
  {
    this._buffer.push(val);
  }

  toString()
  {
    return this._buffer.join("");
  }
}

class Svg {
  static get NS() { return "http://www.w3.org/2000/svg"; }

  static findElement(element, elementName)
  {
    let elements = element.getElementsByTagNameNS(Svg.NS, elementName)

    if (0 < elements.length)
      return elements[0];
    else
      return null;
  }

  static findOrCreateElement(element, elementName)
  {
    let e = Svg.findElement(element, elementName);

    if (e)
      return e
    else
      return element.appendChild(Svg.createElement(elementName, null, null));
  }

  static createElement(elementName, attributes, innerText)
  {
    let e = document.createElementNS(Svg.NS, elementName);

    if (innerText)
      e.appendChild(document.createTextNode(innerText));

    if (!attributes)
      return e;

    for (let attr in attributes) {
      if (!(attributes.hasOwnProperty(attr) && attributes[attr]))
        continue;

      let val = attributes[attr];

      if (Object.prototype.toString.call(val) === "[object Object]") {
        let delimAt = attr.indexOf(":");
        let nsPrefix = attr.substring(0, delimAt);
        let attrName = attr.substring(delimAt + 1);
        let nsUri = val.nsuri;
        let attrVal = val.val;

        e.setAttributeNS(nsUri, attrName, attrVal);
        e.setAttributeNS("http://www.w3.org/2000/xmlns/", "xmlns:" + nsPrefix, nsUri);
      }
      else {
        e.setAttribute(attr, val);
      }
    }

    return e;
  }

  static createTextElement(attributes, innerText)
  {
    let e = Svg.createElement("text", attributes, innerText);

    // TODO: multiline text
    /*
    innerText.split(/\r\n|\r|\n/).forEach((text, line) => {
      e.appendChild(Svg.createElement("tspan", {"dy": line + "em"}, text));
    });
    */

    return e;
  }

  static getAnimationElements(element)
  {
    return Array.prototype.concat.call(
      Array.prototype.slice.call(element.getElementsByTagNameNS(Svg.NS, "animate")),
      Array.prototype.slice.call(element.getElementsByTagNameNS(Svg.NS, "animateMotion"))
    );
  }

  static minimizeElement(svgElement, padding, maxWidth, maxHeight)
  {
    if (svgElement.ownerSVGElement)
      svgElement = svgElement.ownerSVGElement;

    let l = 0, t = 0, r = 0, b = 0;

    for (let i = 0; i < svgElement.children.length; i++) {
      if (!svgElement.children[i].getBBox)
        continue; // not SVGLocatable
      if (svgElement.children[i].localName === "defs")
        continue; // ignore <defs>

      let box = svgElement.children[i].getBBox();

      if (l > box.x) l = box.x;
      if (t > box.y) t = box.y;

      let br = box.x + box.width;
      let bb = box.y + box.height;

      if (r < br) r = br;
      if (b < bb) b = bb;
    }

    if (isNaN(padding))
      padding = 0.0;

    let viewBox = {
      x: l - padding,
      y: t - padding,
      width:  (r - l) + padding * 2.0,
      height: (b - t) + padding * 2.0,
    };
    let viewportWidth = viewBox.width;
    let viewportHeight = viewBox.height;

    if (maxWidth || maxHeight) {
      if (maxWidth < viewportWidth) {
        viewportHeight = viewportHeight * (maxWidth / viewportWidth);
        viewportWidth = maxWidth;
      }
      else if (maxHeight < viewportHeight) {
        viewportWidth = viewportWidth * (maxHeight / viewportHeight);
        viewportHeight = maxHeight;
      }
    }

    let viewportElement = svgElement.viewportElement ? svgElement.viewportElement : svgElement;

    viewportElement.setAttribute("width", viewportWidth);
    viewportElement.setAttribute("height", viewportHeight);
    viewportElement.setAttribute("viewBox", `${viewBox.x} ${viewBox.y} ${viewBox.width} ${viewBox.height}`);
  }

  static clientPointToViewportPoint(svgElement, clientX, clientY)
  {
    let pt = svgElement.createSVGPoint();

    pt.x = clientX;
    pt.y = clientY;

    if (false) {
      return pt.matrixTransform(svgElement.getCTM().inverse()); // getCTM() returns null with Firefox
    }
    else {
      let viewportElement = svgElement.viewportElement ? svgElement.viewportElement : svgElement;
      let rect = svgElement.getBoundingClientRect();

      pt.x += rect.left;
      pt.y += rect.top;

      return pt.matrixTransform(viewportElement.getScreenCTM().inverse());
    }
  }
}

class ExpressionTreeNode extends Node {
  constructor(expression, left, right)
  {
    super(expression);

    this.left = left;
    this.right = right;
  }

  get postorderNotation()
  {
    let stdout = new PseudoStdOut();

    this.traversePostorder(stdout);

    return stdout.toString().trim();
  }

  get inorderNotation()
  {
    let stdout = new PseudoStdOut();

    this.traverseInorder(stdout);

    return stdout.toString().trim();
  }

  get preorderNotation()
  {
    let stdout = new PseudoStdOut();

    this.traversePreorder(stdout);

    return stdout.toString().trim();
  }

  createVisualTree()
  {
    return ExpressionTreeNode._createVisualTree(this);
  }

  static _createVisualTree(node)
  {
    return new VisualTreeNode(node,
                              node.left  ? ExpressionTreeNode._createVisualTree(node.left)  : null,
                              node.right ? ExpressionTreeNode._createVisualTree(node.right) : null);
  }
}

class VisualTreeNode {
  static get RADIUS() { return 12.5; }
  static get SIBLING_DISTANCE() { return /*1.0*/ 0.95; }
  static get BRANCH_WIDTH() { return VisualTreeNode.RADIUS * 1.25; }
  static get BRANCH_HEIGHT() { return VisualTreeNode.RADIUS * 5.0; }

  get inorderNotation()
  {
    let stdout = new PseudoStdOut();

    this.node.traverseInorder(stdout);

    return stdout.toString();
  }

  get description() { return this.inorderNotation; };
  get value() { return this.node.expression; }
  get nodeBound() { return this.nodeElement ? this.nodeElement.getBBox() : null; };
  get treeBound() { return this.nodeElement ? this.nodeElement.parentNode.getBBox() : null; };
  get depth() { return 1 + Math.max(this.left ? this.left.depth : 0, this.right ? this.right.depth : 0); };

  constructor(node, subTreeLeft, subTreeRight)
  {
    this.node = node;
    this.left = subTreeLeft;
    this.right = subTreeRight;
    this.nodeElement = null;
    this.x = 0;
    this.y = 0;
  }

  static createSvgElement(id)
  {
    return VisualTreeNode.initializeSvgElement(Svg.createElement("svg", {
      "id": id,
      "version": "1.1",
      "xmlns": Svg.NS,
    }));
  }

  static initializeSvgElement(e)
  {
    e.classList.add("expressiontree");
    e.setAttribute("width", "300"); // as default
    e.setAttribute("height", "300"); // as default
    e.setAttribute("viewBox", "-150 -40 300 300"); // as default

    return e;
  }

  /*
   * rendering
   */

  render(target, renderInformationalElements)
  {
    this.locate();

    return this._render(target, renderInformationalElements);
  }

  locate()
  {
    this._locate(0, 0);
  }

  renderSubTree(target, level, depthMax, renderInformationalElements)
  {
    return this._render(target, renderInformationalElements);
  }

  _locate(x, y)
  {
    this.x = x;
    this.y = y;

    let dxl = VisualTreeNode.BRANCH_WIDTH * Math.pow(VisualTreeNode.SIBLING_DISTANCE * 2, this._getLeftWidthScaler());
    let dxr = VisualTreeNode.BRANCH_WIDTH * Math.pow(VisualTreeNode.SIBLING_DISTANCE * 2, this._getRightWidthScaler());
    let dy = VisualTreeNode.BRANCH_HEIGHT;

    if (this.left)
      this.left._locate(this.x - dxl, this.y + dy, null, null);

    if (this.right)
      this.right._locate(this.x + dxr, this.y + dy, null, null);
  }

  _getLeftWidthScaler()
  {
    if (false && this.left)
      return this.left.depth;
    else
      return this.depth - 1;
  }

  _getRightWidthScaler()
  {
    if (false && this.right)
      return this.right.depth;
    else
      return this.depth - 1;
  }

  _render(target, renderInformationalElements)
  {
    let groupSubtree = target.appendChild(Svg.createElement("g", {"class": "expressiontree-subtree"}));

    groupSubtree.appendChild(Svg.createElement("desc", null, "subtree of '" + this.description + "'"));

    [
      {tree: this.left,  className: "expressiontree-subtree expressiontree-subtree-left"},
      {tree: this.right, className: "expressiontree-subtree expressiontree-subtree-right"},
    ]
    .forEach((subtree) => {
      if (!subtree.tree)
        return;

      groupSubtree.appendChild(Svg.createElement("line", {
        "class": "expressiontree-branch",
        "x1": this.x, "x2": subtree.tree.x,
        "y1": this.y, "y2": subtree.tree.y,
      }));

      let t = subtree.tree._render(groupSubtree, renderInformationalElements);

      t.setAttribute("class", subtree.className);
    });

    let groupNode = groupSubtree.appendChild(Svg.createElement("g", {"class": "expressiontree-node"}));

    let label = groupNode.appendChild(Svg.createTextElement({
      "class": "expressiontree-node-label",
      "x": this.x,
      "y": this.y,
      "text-anchor": "middle",
      "dominant-baseline": "middle",
    }, this.value));

    const nodeBodyWidth  = Math.max(VisualTreeNode.RADIUS * 2.0, label.getBBox().width * 1.25);
    const nodeBodyHeight = VisualTreeNode.RADIUS * 2.0;

    let nodeBody = groupNode.appendChild(Svg.createElement("rect", {
      "class": "expressiontree-node-body",
      "x": this.x - nodeBodyWidth / 2.0,
      "y": this.y - nodeBodyHeight / 2.0,
      "width": nodeBodyWidth,
      "height": nodeBodyHeight,
      "rx": VisualTreeNode.RADIUS,
      "ry": VisualTreeNode.RADIUS,
    }));

    groupNode.insertBefore(label, nodeBody.nextSibling); // move label to front

    this.nodeElement = groupNode;

    if (renderInformationalElements && (this.left && this.right))
      this.renderInformation(groupSubtree, groupSubtree.firstChild /*<desc>*/, this.description);

    return groupSubtree;
  }

  renderInformation(target, refNode, informationalText)
  {
    let bound = target.getBBox();
    let ex = VisualTreeNode.RADIUS * 0.8;
    let ey = VisualTreeNode.RADIUS * 0.5;

    let r = target.insertBefore(Svg.createElement("rect", {
      "class": "expressiontree-subtree-bound",
      "x": bound.x - ex,
      "y": bound.y - ey,
      "width":  bound.width  + ex * 2,
      "height": bound.height + ey * 2,
      "rx": VisualTreeNode.RADIUS,
      "ry": VisualTreeNode.RADIUS,
    }), refNode.nextSibling);

    target.insertBefore(Svg.createElement("text", {
      "class": "expressiontree-subtree-label",
      "x": this.x,
      "y": this.y - VisualTreeNode.RADIUS * 2.0,
      "text-anchor": "middle",
      "dominant-baseline": "text-after-edge",
    }, informationalText), r.nextSibling);
  }

  getNodeAnchorPoint(scale, radian)
  {
    return {
      x: this.x + (this.nodeBound.width  / 2.0) * scale * Math.cos(radian),
      y: this.y + (this.nodeBound.height / 2.0) * scale * Math.sin(radian),
    };
  }

  /*
   * traversing and paths
   */

  traversePathInorder(traverser)
  {
    this._traversePathInorder(traverser);

    return traverser.closePath();
  }

  traversePathPreorder(traverser)
  {
    this._traversePathPreorder(traverser);

    return traverser.closePath();
  }

  traversePathPostorder(traverser)
  {
    this._traversePathPostorder(traverser);

    return traverser.closePath();
  }

  static get PATH_THIS_TO_LEFT() { return 0; }
  static get PATH_LEFT_TO_THIS() { return 1; }
  static get PATH_LEFT_TO_RIGHT() { return 2; }
  static get PATH_THIS_TO_RIGHT() { return 3; }
  static get PATH_RIGHT_TO_THIS() { return 4; }
  static get PATH_SELF_PREORDER() { return 5; }
  static get PATH_SELF_INORDER() { return 6; }
  static get PATH_SELF_POSTORDER() { return 7; }

  _traversePathInorder(traverser)
  {
    if (this.left) {
      this._traversePathThisToLeft(traverser);

      this.left._traversePathInorder(traverser);

      this._traversePathLeftToThis(traverser);
    }

    VisualTreeNode._traversePath(traverser, this, this, VisualTreeNode.PATH_SELF_INORDER);

    if (this.right) {
      this._traversePathThisToRight(traverser);

      this.right._traversePathInorder(traverser);

      this._traversePathRightToThis(traverser);
    }
  }

  _traversePathPreorder(traverser)
  {
    VisualTreeNode._traversePath(traverser, this, this, VisualTreeNode.PATH_SELF_PREORDER);

    if (this.left) {
      this._traversePathThisToLeft(traverser);

      this.left._traversePathPreorder(traverser);

      this._traversePathLeftToThis(traverser);
    }

    if (this.right) {
      this._traversePathThisToRight(traverser);

      this.right._traversePathPreorder(traverser);

      this._traversePathRightToThis(traverser);
    }
  }

  _traversePathPostorder(traverser)
  {
    if (this.left) {
      this._traversePathThisToLeft(traverser);

      this.left._traversePathPostorder(traverser);

      this._traversePathLeftToThis(traverser);
    }

    if (this.right) {
      this._traversePathThisToRight(traverser);

      this.right._traversePathPostorder(traverser);

      this._traversePathRightToThis(traverser);
    }

    VisualTreeNode._traversePath(traverser, this, this, VisualTreeNode.PATH_SELF_POSTORDER);
  }

  _traversePathThisToLeft(traverser)
  {
    VisualTreeNode._traversePath(traverser, this, this.left, VisualTreeNode.PATH_THIS_TO_LEFT);
  }

  _traversePathLeftToThis(traverser)
  {
    VisualTreeNode._traversePath(traverser, this.left, this, VisualTreeNode.PATH_LEFT_TO_THIS);
  }

  _traversePathLeftToRight(traverser)
  {
    VisualTreeNode._traversePath(traverser, this.left, this.right, VisualTreeNode.PATH_LEFT_TO_RIGHT);
  }

  _traversePathThisToRight(traverser)
  {
    VisualTreeNode._traversePath(traverser, this, this.right, VisualTreeNode.PATH_THIS_TO_RIGHT);
  }

  _traversePathRightToThis(traverser)
  {
    VisualTreeNode._traversePath(traverser, this.right, this, VisualTreeNode.PATH_RIGHT_TO_THIS);
  }

  static _traversePath(traverser, nodeFrom, nodeTo, pathType)
  {
    // [node point]
    //        0
    //    1   *   5
    //     *     *
    //     *     *
    //    2   *   4
    //        3
    let nodePointFrom = 0;
    let nodePointTo = 0;
    let outwardArc = true;

    if (pathType == VisualTreeNode.PATH_THIS_TO_LEFT) {
      nodePointFrom = 1;
      nodePointTo   = 0;
    }
    else if (pathType == VisualTreeNode.PATH_LEFT_TO_THIS) {
      nodePointFrom = 4.5;
      nodePointTo   = 2.5;
    }
    else if (pathType == VisualTreeNode.PATH_LEFT_TO_RIGHT) {
      nodePointFrom = 4.5;
      nodePointTo   = 1.5;
      outwardArc = false;
    }
    else if (pathType == VisualTreeNode.PATH_THIS_TO_RIGHT) {
      nodePointFrom = 3.5;
      nodePointTo   = 1.5;
    }
    else if (pathType == VisualTreeNode.PATH_RIGHT_TO_THIS) {
      nodePointFrom = 0;
      nodePointTo   = 5;
    }
    else if (pathType == VisualTreeNode.PATH_SELF_PREORDER) {
      nodePointFrom = 2;
      nodePointTo   = 2;
    }
    else if (pathType == VisualTreeNode.PATH_SELF_INORDER) {
      nodePointFrom = 3;
      nodePointTo   = 3;
    }
    else if (pathType == VisualTreeNode.PATH_SELF_POSTORDER) {
      nodePointFrom = 4;
      nodePointTo   = 4;
    }

    const radNodePointFrom = -Math.PI * ((nodePointFrom / 3.0) + 0.5);
    const radNodePointTo   = -Math.PI * ((nodePointTo   / 3.0) + 0.5);
    const posFrom = nodeFrom.getNodeAnchorPoint(1.1, radNodePointFrom);
    const posTo   = nodeTo  .getNodeAnchorPoint(1.1, radNodePointTo);

    let d = "";

    if (nodePointFrom === nodePointTo) {
      const r = VisualTreeNode.RADIUS / 2.0;
      const rc = nodeFrom.getNodeAnchorPoint(1.0, radNodePointFrom);
      const radPath  = radNodePointFrom;
      const radStart = radPath + Math.PI * (+1 / 6);
      const radEnd   = radPath + Math.PI * (-1 / 6);

      d = "M ";
      d += (rc.x + r * Math.cos(radStart)) + " ";
      d += (rc.y + r * Math.sin(radStart)) + " ";

      d += "A ";
      d += r * 1.5 + " "; // rx
      d += r * 1.0 + " "; // ry
      d += (radPath * 180 / Math.PI) + " "; // x-axis-rotation
      d += "1 "; // large-arc-flag
      d += "0 "; // sweep-flag
      d += (rc.x + r * Math.cos(radEnd)) + " ";
      d += (rc.y + r * Math.sin(radEnd)) + " ";
    }
    else {
      d = "M ";
      d += posFrom.x + " "; d += posFrom.y + " ";

      if (outwardArc) {
        d += "C ";

        const dx = posFrom.x - posTo.x;
        const dy = posFrom.y - posTo.y;
        const rad = Math.atan2(dy, dx) - Math.PI / 2;
        const len = Math.sqrt(dx * dx + dy * dy) / 4.0;
        const arg = Math.PI / 4;

        // x1, y1
        d += (posFrom.x + len * Math.cos(rad - arg)) + " ";
        d += (posFrom.y + len * Math.sin(rad - arg)) + " ";

        // x2, y2
        d += (posTo.x + len * Math.cos(rad + arg)) + " ";
        d += (posTo.y + len * Math.sin(rad + arg)) + " ";
      }
      else {
        d += "Q ";

        // x1, y1
        d += ((posFrom.x + posTo.x) / 2.0) + " ";
        d += ((posFrom.y + posTo.y) / 2.0) - VisualTreeNode.BRANCH_HEIGHT + " ";
      }

      d += " ";
      d += posTo.x + " "; d += posTo.y + " ";
    }

    traverser.appendPath(nodeFrom, nodeTo, d);
  }

  /*
   * calculation
   */

  calculate(transitionProcessor)
  {
    let tree = VisualTreeNode._cloneTree(this);

    transitionProcessor.startTransition(tree);

    VisualTreeNode._calculate(tree, tree, transitionProcessor);

    transitionProcessor.endTransition(tree);
  }

  static _calculate(root, subTree, transitionProcessor)
  {
    if (subTree.left)
      VisualTreeNode._calculate(root, subTree.left, transitionProcessor);

    if (subTree.right)
      VisualTreeNode._calculate(root, subTree.right, transitionProcessor);

    if (!(subTree.left && subTree.right))
      return;

    transitionProcessor.setTreeBeforeTransition(root, subTree);

    if (!subTree.node.calculate()) {
      subTree.node.expression = subTree.inorderNotation;
      subTree.node.left  = null;
      subTree.node.right = null;
    }

    subTree.left = null;
    subTree.right = null;

    transitionProcessor.setTreeAfterTransition(root, subTree);
  }

  static _cloneTree(n)
  {
    let clone = new VisualTreeNode(new Node(n.node.expression),
                                   n.left  ? VisualTreeNode._cloneTree(n.left)  : null,
                                   n.right ? VisualTreeNode._cloneTree(n.right) : null);

    clone.x = n.x;
    clone.y = n.y;

    if (clone.left)
      clone.node.left = clone.left.node;

    if (clone.right)
      clone.node.right = clone.right.node;

    return clone;
  }
}

class TreeTraverser {
  appendPath(nodeFrom, nodeTo, d) {}
  closePath() {}
}

class TraversalPathRenderer extends TreeTraverser {
  constructor(target, targetDefs)
  {
    super();

    this.target = target;
    this.targetDefs = targetDefs;
  }

  static _renderPathMarker(target)
  {
    const markerId = "polish-path-marker-arrow";

    if (!target)
      return;
    if (target.querySelector("#" + markerId))
      return;

    let defs = Svg.findOrCreateElement(target, "defs");

    let m = defs.appendChild(Svg.createElement("marker", {
      "class": "polish-path-marker",
      "id": markerId,
      "markerUnits": "strokeWidth",
      "markerWidth": "5",
      "markerHeight": "5",
      "viewBox": "-5 -5 10 10",
      "refX": "5",
      "refY": "0",
      "orient": "auto",
    }));

    m.appendChild(Svg.createElement("polygon", {
      "points": "+5,0 -5,-5, -2,0 -5,+5",
    }));

    return m;
  }

  appendPath(nodeFrom, nodeTo, d)
  {
    let p = Svg.createElement("path", {
      "class": "polish-path",
      "d": d,
      "marker-end": "url(#polish-path-marker-arrow)"
    });

    this.target.appendChild(p);
  }

  closePath()
  {
    TraversalPathRenderer._renderPathMarker(this.targetDefs);

    return null; // TODO
  }
}

class TraversalOrderRenderer extends TreeTraverser {
  static get DURATION_IN_SECS() { return 1.0; }
  static get WAIT_IN_SECS() { return 2.5; }
  static get T_OFFSET() { return 0.0; }
  static get T_OFFSET_STATUSREAD() { return TraversalOrderRenderer.DURATION_IN_SECS * 0.6; }

  static get _ID_PATH_MARKER() { return "expressiontree-traversalorder-path-marker-arrow";}

  get totalDurationInSeconds() { return this.t; }

  constructor(target, targetDefs)
  {
    super();

    this.target = target;
    this.targetDefs = targetDefs;
    this.t = TraversalOrderRenderer.T_OFFSET;
    this.order = 0;
    this.groupRoot = null;
    this.groupOrder = null;
    this.paths = [];
    this.nodeStatuses = [];
    this.orderedNodes = [];
  }

  static _renderPathSymbol(target)
  {
    if (!target)
      return;
    if (target.querySelector("#" + TraversalOrderRenderer._ID_PATH_MARKER))
      return;

    let defs = Svg.findOrCreateElement(target, "defs");

    let s = defs.appendChild(Svg.createElement("symbol", {
      "class": "expressiontree-traversalorder-path-marker",
      "id": TraversalOrderRenderer._ID_PATH_MARKER,
      "viewBox": "-5 -5 10 10",
    }));

    s.appendChild(Svg.createElement("polygon", {
      "points": "+5,0 -5,-5, -2,0 -5,+5",
    }));

    return s;
  }

  appendPath(nodeFrom, nodeTo, d)
  {
    if (!this.groupRoot) {
      let nodeRoot = nodeFrom;

      this.groupRoot = this.target.appendChild(Svg.createElement("g", {"class": "expressiontree-traversalorder"}));
      this.groupRoot.appendChild(Svg.createElement("desc", null, "traversal order of " + nodeFrom.description));

      this.groupOrder = this.groupRoot.appendChild(Svg.createElement("g"));
      this.groupOrder.appendChild(Svg.createElement("desc", null, "traversed expressions"));

      this.initializeNodeStatus(nodeRoot, this.t);
    }

    this.paths.push({t: this.t, d: d, order: this.order});

    this.initializeNodeStatus(nodeFrom);
    this.initializeNodeStatus(nodeTo, this.t);

    if (nodeFrom === nodeTo) {
      this.order += 1;

      this.getNodeStatus(nodeFrom).readAt = this.t;

      this.orderedNodes.push({t: this.t, order: this.order, node: nodeFrom});
    }

    this.t += TraversalOrderRenderer.DURATION_IN_SECS;
  }

  closePath()
  {
    TraversalOrderRenderer._renderPathSymbol(this.targetDefs);

    let dur = this.totalDurationInSeconds + TraversalOrderRenderer.WAIT_IN_SECS;

    this.paths.forEach((path) => TraversalOrderRenderer.renderPath(this.groupRoot, path.t, dur, path.d, path.order));

    this.nodeStatuses.forEach((status) => TraversalOrderRenderer.renderNodeStatus(this.groupRoot, status, dur));

    let right = 0;

    this.orderedNodes.forEach((orderedNode) => {
      let pointOrder = orderedNode.node.getNodeAnchorPoint(2.5, Math.PI / 2);

      TraversalOrderRenderer.renderOrder(this.groupRoot,
                                         orderedNode.t,
                                         dur,
                                         orderedNode.order,
                                         pointOrder.x,
                                         pointOrder.y,
                                         orderedNode.node.value,
                                         true);

      TraversalOrderRenderer.renderOrder(this.groupOrder,
                                         orderedNode.t,
                                         dur,
                                         orderedNode.order,
                                         0,
                                         this.groupOrder.getBBox().height,
                                         orderedNode.node.value,
                                         false);

      right = Math.max(right, orderedNode.node.x);
    });

    this.groupOrder.setAttribute("transform",
                                 "translate(" + (right + VisualTreeNode.RADIUS * 1.5 + this.groupOrder.getBBox().width) + ",0)");

    return this.groupRoot;
  }

  static renderPath(target, t, dur, d, order)
  {
    const begin = t + "s";
    const duration = dur + "s";
    const tr = TraversalOrderRenderer.DURATION_IN_SECS / dur;
    const keyTimesDiscrete = [0, tr, 1].join(";");
    const keyTimesSpline = [0, tr, (tr + 0.0001), 1].join(";");
    const keySplines = ["0.25 0.1 0.25 1.0", "0.0 0.0 1.0 1.0", "0.0 0.0 1.0 1.0"].join("; ");

    let g = target.appendChild(Svg.createElement("g", {
      "class": "expressiontree-traversalorder-path",
      "visibility": "hidden",
    }));

    g.appendChild(Svg.createElement("desc", null, "path of #" + order));

    let pathPath = g.appendChild(Svg.createElement("path", {
      "d": d,
      "pathLength": "100", // % of total length
      "stroke-dasharray": "100", // %
      "stroke-dashoffset": "100", // %
    }));

    pathPath.appendChild(Svg.createElement("animate", {
      "attributeName": "stroke-dashoffset",
      "begin": begin,
      "dur": duration,
      "fill": "remove",
      "repeatCount": "indefinite",
      // easing
      "calcMode": "spline",
      "keyTimes": keyTimesSpline,
      "keySplines": keySplines,
      "values": "100;0;100;100", // %
    }));

    let pathMarker = g.appendChild(Svg.createElement("use", {
      "xlink:href": {nsuri: "http://www.w3.org/1999/xlink", val: "#" + TraversalOrderRenderer._ID_PATH_MARKER},
      "x": "-5",
      "y": "-5",
      "width": "10",
      "height": "10",
    }));

    pathMarker.appendChild(Svg.createElement("animateMotion", {
      "path": d,
      "begin": begin,
      "dur": duration,
      "rotate": "auto",
      "fill": "remove",
      "repeatCount": "indefinite",
      // easing
      "calcMode": "spline",
      "keyTimes": keyTimesSpline,
      "keySplines": keySplines,
      "keyPoints": "0;1;1;1",
    }));

    // append <animate>
    g.appendChild(Svg.createElement("animate", {
      "attributeName": "visibility",
      "begin": begin,
      "dur": duration,
      "fill": "freeze",
      "repeatCount": "indefinite",
      // easing
      "calcMode": "discrete",
      "keyTimes": keyTimesDiscrete,
      "values": "visible;hidden;hidden",
    }));

    return g;
  }

  static renderOrder(target, t, dur, order, x, y, contentText, centering)
  {
    let g = target.appendChild(Svg.createElement("g", {
      "class": "expressiontree-traversalorder-order",
      "visibility": "hidden",
    }));

    g.appendChild(Svg.createElement("desc", null, "#" + order + ": '" + contentText + "'"));

    let content = g.appendChild(Svg.createElement("text", {
      "class": "expressiontree-traversalorder-order-content",
      "x": x,
      "y": y,
      "text-anchor": centering ? "middle" : "start",
      "dominant-baseline": "middle",
    }, contentText));

    let contentBound = content.getBBox();
    const contentPaddingX = 6;
    const contentPaddingY = 2;
    let contentWidth  = Math.max(30, contentBound.width + contentPaddingX * 2);
    let contentHeight = contentBound.height + contentPaddingY * 2.0;

    let boxContent = Svg.createElement("rect", {
      "class": "expressiontree-traversalorder-order-content-box",
      "x": x + (centering ? -contentWidth / 2.0 : -contentPaddingX),
      "y": y - contentHeight / 2.0,
      "width": contentWidth,
      "height": contentHeight,
    });

    g.appendChild(boxContent);
    g.insertBefore(content, boxContent.nextSibling);

    let boxContentBound = boxContent.getBBox();

    const labelPadding = 2;

    let label = g.appendChild(Svg.createElement("text", {
      "class": "expressiontree-traversalorder-order-label",
      "x": boxContentBound.x - 2,
      "y": y,
      "text-anchor": "end",
      "dominant-baseline": "middle",
    }, order.toString()));

    let labelBound = label.getBBox();
    let labelWidth = Math.max(20, labelBound.width + labelPadding * 2);

    let boxLabel = Svg.createElement("rect", {
      "class": "expressiontree-traversalorder-order-label-box",
      "x": boxContentBound.x - labelWidth,
      "y": boxContentBound.y,
      "width": labelWidth,
      "height": boxContentBound.height,
    });

    g.appendChild(boxLabel);
    g.insertBefore(label, boxLabel.nextSibling);

    // bring to front
    g.insertBefore(boxContent, label.nextSibling);
    g.insertBefore(content, boxContent.nextSibling);

    // append <animate>
    const startAt = TraversalOrderRenderer.T_OFFSET;
    let tt = t + TraversalOrderRenderer.T_OFFSET_STATUSREAD - startAt;

    g.appendChild(Svg.createElement("animate", {
      "attributeName": "visibility",
      "begin": startAt + "s",
      "dur": dur + "s",
      "fill": "freeze",
      "repeatCount": "indefinite",
      // easing
      "calcMode": "discrete",
      "keyTimes": [0, (tt / dur), (dur - TraversalOrderRenderer.WAIT_IN_SECS) / dur, 1].join(";"),
      "values": "hidden;visible;visible;hidden",
    }));

    return g;
  }

  initializeNodeStatus(n, reachedAt)
  {
    let s = this.getNodeStatus(n);

    if (s) {
      return s;
    }
    else {
      let ss = {node: n};

      if (reachedAt)
        ss.reachedAt = reachedAt;

      this.nodeStatuses.push(ss);

      return ss;
    }
  }

  getNodeStatus(n)
  {
    return this.nodeStatuses.find((e) => Object.is(e.node, n));
  }

  static renderNodeStatus(target, status, dur)
  {
    let e = target.appendChild(Svg.createElement("ellipse", {
      "class": "expressiontree-traversalorder-nodemask",
      "cx": status.node.x,
      "cy": status.node.y,
      "rx": status.node.nodeBound.width / 2.0,
      "ry": status.node.nodeBound.height / 2.0,
      "fill": "transparent",
      "stroke": "none",
    }));

    const colorInitial = "transparent";
    const colorReached = "#f007";
    const colorRead = "#888c";
    const startAt = TraversalOrderRenderer.T_OFFSET;
    let keyTimes = [0];
    let values = [];

    if (startAt < status.reachedAt) {
      values.push(colorInitial);

      keyTimes.push((status.reachedAt - startAt) / dur);
    }

    values.push(colorReached);

    keyTimes.push((status.readAt + TraversalOrderRenderer.T_OFFSET_STATUSREAD - startAt) / dur);
    values.push(colorRead);

    keyTimes.push((dur - TraversalOrderRenderer.WAIT_IN_SECS) / dur);
    values.push(colorInitial);

    e.appendChild(Svg.createElement("animate", {
      "attributeName": "fill",
      "begin": startAt + "s",
      "dur": dur + "s",
      "fill": "freeze",
      "repeatCount": "indefinite",
      // easing
      "calcMode": "discrete",
      "keyTimes": keyTimes.join(";"),
      "values": values.join(";"),
    }));
  }
}

class TreeTransitionProcessor {
  constructor() {}

  startTransition(tree) {}
  setTreeBeforeTransition(tree, subTreeTransition) {}
  setTreeAfterTransition(tree, subTreeTransition) {}
  endTransition(tree) {}
}

class CalculationTransitionRenderer extends TreeTransitionProcessor {
  constructor(target, stepsToRender)
  {
    super();

    this.target = target;
    this.rootContainer = null;
    this.frames = [];
    this.stepsToRender = stepsToRender;
  }

  startTransition(tree)
  {
    // render initial tree
    this.rootContainer = this.target.appendChild(Svg.createElement("g", {
      "class": "polosh-calculation",
    }));

    let g = this.rootContainer.insertBefore(Svg.createElement("g"), this.rootContainer.firstChild);

    g.appendChild(Svg.createElement("desc", null, "initial expression tree"));

    tree.locate();

    this.frames.push({
      container: g,
      tree: tree.renderSubTree(g),
    });
  }

  setTreeBeforeTransition(tree, subTreeTransition)
  {
    let g = this.rootContainer.insertBefore(Svg.createElement("g"), this.rootContainer.firstChild);

    g.appendChild(Svg.createElement("desc", null, "expression tree after calculation step #" + this.frames.length));

    let treeBound = subTreeTransition.treeBound;
    const padding = 10;

    let bound = g.appendChild(Svg.createElement("rect", {
      "class": "expressiontree-calculation-subtreebound",
      "x": treeBound.x - padding,
      "y": treeBound.y - padding,
      "width":  treeBound.width  + padding * 2,
      "height": treeBound.height + padding * 2,
      "rx": VisualTreeNode.RADIUS,
      "ry": VisualTreeNode.RADIUS,
      "visibility": "hidden",
    }));

    let subexpressionContentText =
      "= '" +
      subTreeTransition.left.inorderNotation +
      " " +
      subTreeTransition.node.expression +
      " " +
      subTreeTransition.right.inorderNotation +
      "'";

    let subexpression = g.appendChild(Svg.createElement("text", {
      "class": "expressiontree-calculation-subexpression",
      "x": subTreeTransition.x,
      "y": treeBound.y + treeBound.height + 10,
      "text-anchor": "middle",
      "dominant-baseline": "text-before-edge",
      "visibility": "hidden",
    }, subexpressionContentText));

    this.frames.push({
      container: g,
      bound: bound,
      subexpression: subexpression,
    });
  }

  setTreeAfterTransition(tree, subTreeTransition)
  {
    let lastFrame = this.frames[this.frames.length - 1];

    lastFrame.tree = tree.renderSubTree(lastFrame.container);
  }

  endTransition(tree)
  {
    let frames = this.frames;

    if (this.stepsToRender) {
      frames = [];

      this.frames.forEach((f, step) => {
        if (this.stepsToRender.includes(step))
          frames.push(f);
        else
          f.container.parentNode.removeChild(f.container);
      });
    }

    // set animation
    const stepCount = frames.length;
    const begin = "0s";
    const durationPerStep = 3.0;
    const waitStartInSeconds = 0.5;
    const waitEndInSeconds   = 1.5;
    const durationBoundOffset = -(4.0 * durationPerStep / 6.0);
    const durationSubExpressionOffset = -(2.0 * durationPerStep / 6.0);
    const totalDurationInSeconds = waitStartInSeconds + (stepCount * durationPerStep) + waitEndInSeconds;
    const dur = totalDurationInSeconds + "s";
    let t = waitStartInSeconds;

    frames.forEach((frame, step) => {
      let tn = t + durationPerStep

      if (step === 0) {
        if (frame.bound) {
          frame.bound.parentNode.removeChild(frame.bound);
          frame.bound = null;
        }

        if (frame.subexpression) {
          frame.subexpression.parentNode.removeChild(frame.subexpression);
          frame.subexpression = null;
        }
      }

      if (frame.bound) {
        const t0 = (t  + durationBoundOffset) / totalDurationInSeconds;
        const t1 = (tn + durationBoundOffset) / totalDurationInSeconds;

        // set animation for visibility of calculation bound
        frame.bound.appendChild(Svg.createElement("animate", {
          "attributeName": "visibility",
          "begin": begin,
          "dur": dur,
          "fill": "freeze",
          "repeatCount": "indefinite",
          // easing
          "calcMode": "discrete",
          "keyTimes": [0, t0, t1, 1].join(";"),
          "values": ["hidden", "visible", "hidden", "hidden"].join(";"),
        }));
      }

      if (frame.subexpression) {
        const t0 = (t  + durationSubExpressionOffset) / totalDurationInSeconds;
        const t1 = (tn + durationBoundOffset)         / totalDurationInSeconds;

        // set animation for visibility of subexpression
        frame.subexpression.appendChild(Svg.createElement("animate", {
          "attributeName": "visibility",
          "begin": begin,
          "dur": dur,
          "fill": "freeze",
          "repeatCount": "indefinite",
          // easing
          "calcMode": "discrete",
          "keyTimes": [0, t0, t1, 1].join(";"),
          "values": ["hidden", "visible", "hidden", "hidden"].join(";"),
        }));
      }

      if (frame.tree) {
        const t0 = t  / totalDurationInSeconds;
        const t1 = tn / totalDurationInSeconds;
        const keyTimes = [0, t0, t1, /*(t1 + 0.0001),*/ 1].join(";");
        //const keySplines = ["0.0 0.0 1.0 1.0", "0.25 0.1 0.25 1.0", "0.0 0.0 1.0 1.0", "0.0 0.0 1.0 1.0"].join("; ");

        // set animation for visibility of subtree
        const visibilityInitial = (step === 0) ? "visible" : "hidden";
        const visibilityTransit = (step === stepCount - 1) ? "visible" : "hidden"

        frame.tree.appendChild(Svg.createElement("animate", {
          "attributeName": "visibility",
          "begin": begin,
          "dur": dur,
          "fill": "freeze",
          "repeatCount": "indefinite",
          // easing
          "calcMode": "discrete",
          "keyTimes": keyTimes,
          "values": [visibilityInitial, "visible", visibilityTransit, /*visibilityTransit,*/ visibilityTransit].join(";"),
        }));
      }

      t = tn;
    });
  }
}

