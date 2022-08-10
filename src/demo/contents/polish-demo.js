function polish_demo_remove_children(e)
{
  while (e.firstChild)
    e.removeChild(e.firstChild);
}

function polish_demo_clear(clearInput)
{
  if (clearInput)
    document.getElementById("polish-demo-input-expression").value = '';

  document.getElementById("polish-demo-conversionresult-expression-postorder").value = '';
  document.getElementById("polish-demo-conversionresult-expression-inorder").value = '';
  document.getElementById("polish-demo-conversionresult-expression-preorder").value = '';
  document.getElementById("polish-demo-conversionresult-calculation").value = '';

  polish_demo_remove_children(document.getElementById("polish-demo-expressiontree-traverse-preorder"));
  polish_demo_remove_children(document.getElementById("polish-demo-expressiontree-traverse-postorder"));
  polish_demo_remove_children(document.getElementById("polish-demo-expressiontree-traverse-inorder"));
  polish_demo_remove_children(document.getElementById("polish-demo-expressiontree-calculation"));

  polish_demo_set_message(null, null);
}

function polish_demo_set_message(message, color)
{
  polish_demo_remove_children(document.getElementById("polish-demo-message"));

  if (message) {
    document.getElementById("polish-demo-message").setAttribute("style", "color:" + color + ";");
    document.getElementById("polish-demo-message").appendChild(document.createTextNode(message));
  }
}

function polish_demo_convert_notation(expression)
{
  let root = new ExpressionTreeNode(expression);

  root.parse();

  document.getElementById("polish-demo-conversionresult-expression-preorder").value  = root.preorderNotation;
  document.getElementById("polish-demo-conversionresult-expression-inorder").value   = root.inorderNotation;
  document.getElementById("polish-demo-conversionresult-expression-postorder").value = root.postorderNotation;

  if (root.calculate()) {
    document.getElementById("polish-demo-conversionresult-calculation").value = root.expression;
    polish_demo_set_message("calculated", "green");
  }
  else {
    document.getElementById("polish-demo-conversionresult-calculation").value = root.inorderNotation;
    polish_demo_set_message("calculated partially", "orange");
  }
}

function polish_demo_visualize(expression)
{
  let root = new ExpressionTreeNode(expression);

  root.parse();

  let visualizations = [
    {
      targetId: "polish-demo-expressiontree-traverse-preorder",
      action: function(tree, target) {
        tree.render(target, true);
        tree.traversePathPreorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      targetId: "polish-demo-expressiontree-traverse-postorder",
      action: function(tree, target) {
        tree.render(target, true);
        tree.traversePathPostorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      targetId: "polish-demo-expressiontree-traverse-inorder",
      action: function(tree, target) {
        tree.render(target, true);
        tree.traversePathInorder(new TraversalOrderRenderer(target, null));
      },
    },
    {
      targetId: "polish-demo-expressiontree-calculation",
      action: function(tree, target) {
        tree.calculate(new CalculationTransitionRenderer(target));
      },
    },
  ];

  visualizations.forEach(function(v) {
    let target = VisualTreeNode.initializeSvgElement(document.getElementById(v.targetId));

    if (typeof smdn !== 'undefined')
      // this is required on https://smdn.jp/programming/tips/polish/
      smdn.tabs.selectByTabContent(target);

    v.action(root.createVisualTree(), target);

    const maxWidth = 1080;

    Svg.minimizeElement(target, 5, maxWidth);

    target.setCurrentTime(0);
    target.unpauseAnimations();
  });

/*
  let i = 0;

  setTimeout(function timeout() {
    if (visualizations.length <= i)
      return;

    let v = visualizations[i++];

    try {
      let target = document.getElementById(v.targetId)
                           .appendChild(VisualTreeNode.createSvgElement());

      selectTabOfContent(target); // XXX

      v.action(root.createVisualTree(), target);

      Svg.minimizeElement(target, 5);

      setTimeout(timeout, 0);
    }
    catch (e) {
      console.log(e);
    }
  }, 0);
*/
}

function polish_demo_run(e)
{
  try {
    e.disabled = true;

    let expression = document.getElementById("polish-demo-input-expression").value;

    polish_demo_clear(false);

    try {
      expression = expression.replace(/\s+/g, '');

      Node.validateBracketBalance(expression);

      polish_demo_convert_notation(expression);

      if (document.getElementById("polish-demo-visualize").checked)
        polish_demo_visualize(expression);
    }
    catch(e) {
      polish_demo_set_message(e.toString(), "red");
    }
  }
  finally {
    e.disabled = false;
  }
}
