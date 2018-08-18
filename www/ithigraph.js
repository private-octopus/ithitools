// Common functions used in the ITHI web pages.

function setDateElement(dateValue) {
    dateElement = document.getElementById("dateHeading");
    dateElement.innerHTML = dateValue;
}

function setValElement(valElementId, x) {
    var elm;

    elm = document.getElementById(valElementId);
    if (x.toFixed(2)) {
        elm.innerHTML = "<b>" + x.toFixed(2) + "%</b>";
    }
    else {
        elm.innerHTML = "<b>" + x + "</b>";
    }
}

function setValElementX(valElementId, x) {
    elm = document.getElementById(valElementId);
    if (x.toFixed(2)) {
        elm.innerHTML = "<b>" + x.toFixed(2) + "</b>";
    }
    else {
        elm.innerHTML = "<b>" + x + "</b>";
    }
}

function getLastElement(dataSet) {
    if (dataSet.length < 1) {
        return 0;
    }
    else {
        return dataSet[dataSet.length - 1];
    }
}

function getAverageElement(dataSet) {
    if (dataSet.length <= 1) {
        return 0;
    }
    else {
        var i = 0;
        var last = 0;
        var first = 0;

        last = dataSet.length - 2;

        if (dataSet.length > 13) {
            first = dataSet.length - 13;
        }
        average = 0;
        for (i = first; i <= last; i++) {
            average += dataSet[i];
        }
        average /= (last + 1 - first);

        return average;
    }
}

function getMaxElement(dataSet) {
    if (dataSet.length < 1) {
        return 0;
    } else {
        var i = 0;

        var maxEl = dataSet[0];

        for (i = 0; i < dataSet.length; i++) {
            if (maxEl < dataSet[i]) {
                maxEl = dataSet[i];
            }
        }

        return maxEl;
    }
}

function getMaxRange(rawMax) {
    var i = 0;
    var t_max = 0.001;

    for (i = 0; i < 6; i++) {
        if (2.0 * t_max > rawMax) {
            return 2.0 * t_max;
        } else if (5.0 * t_max > rawMax) {
            return (5.0 * t_max);
        } else {
            t_max *= 10.0;

            if (t_max > rawMax) {
                return (t_max);
            }
        }
    }

    return rawMax;
}

function columnSum(dataSet, columnIndex) {
    var sum = 0;
    var i = 0;

    for (i = 0; i < dataSet.length; i++) {
        var lineSet = dataSet[i];
        sum += lineSet[columnIndex];
    }

    return sum;
}

function setColorBlob(canvasId, colorValue) {
    var c = document.getElementById(canvasId);
    var ctx = c.getContext("2d");
    var w = c.width / 2;
    var h = c.height / 2;
    ctx.fillStyle = colorValue;
    ctx.fillRect(0, 0, w, h);
}

function plotPieChart(canvasPieId, dataSet, colorSet) {
    var c = document.getElementById(canvasPieId);
    var ctx = c.getContext("2d");
    var colorIndex = 0;
    var alphaRad0 = 1.5;
    var alpha = 1.5;
    var alpha2 = 0;
    var xc = c.width / 2;
    var yc = c.height / 2;
    var margin = 10;
    var radius = yc;
    if (radius > xc) {
        radius = xc;
    }

    if (radius > margin) {
        radius -= margin;
    }
    else {
        radius /= 2;
    }

    for (i = 0; i < dataSet.length; i++) {
        ctx.fillStyle = colorSet[colorIndex];
        alpha2 = alpha + (2 * dataSet[i] / 100);
        ctx.beginPath();
        ctx.moveTo(xc, yc);
        ctx.arc(xc, yc, radius, alpha * Math.PI, alpha2 * Math.PI);
        ctx.fill();
        alpha = alpha2;
        colorIndex++;
        if (colorIndex >= colorSet.length) {
            colorIndex = 0;
        }
    }
}

function fillMetricTable(tableName, tableId, dataSet) {
    var i = 0;

    var tableElem = document.getElementById(tableId);
    var tableText = "<table class=\"metrics\"><tr><th>" + tableName + "</th>";

    tableText += "<th class=\"number\">Current value</th> <th class=\"number\">Average value</th></tr >\n";

    for (i = 0; i < dataSet.length; i++) {
        var j = 0;
        var lineSet = dataSet[i];

        tableText += "<tr><td>" + lineSet[0] + "</td>";

        for (j = 1; j < 3 && j < lineSet.length; j++) {
            tableText += "<td class=\"number\">" + lineSet[j].toFixed(3) + "%</td>";
        }
        tableText += "</tr>\n";
    }
    tableElem.innerHTML += "</table>\n";

    tableElem.innerHTML = tableText;
}

function setScale(canvasId, v_max, sections, unit) {
    var v_min = 0;
    var stepSize = v_max / 10;
    var columnSize = 50;
    var rowSize = 50;
    var margin = 10;
    var xAxis = [" ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
    var graph = new Object();
    var i = 0;
    graph.canvas = document.getElementById(canvasId);
    graph.context = graph.canvas.getContext("2d");
    graph.context.fillStyle = "#808080"
    graph.context.font = "20 pt Verdana"

    graph.yScale = (graph.canvas.height - columnSize - margin) / (v_max - v_min);
    graph.xScale = (graph.canvas.width - rowSize) / sections;

    graph.context.strokeStyle = "#808080"; // color of grid lines
    graph.context.beginPath();
    // print Parameters on X axis, and grid lines on the graph
    for (i = 1; i <= sections + 1; i++) {
        var x = i * graph.xScale;
        graph.context.fillText(xAxis[i], x, columnSize - margin);
        graph.context.moveTo(x, columnSize + margin);
        graph.context.lineTo(x, graph.canvas.height);
    }
    // print row header and draw horizontal grid lines
    var count = 0;
    var scale = 0;
    for (scale = v_max; scale >= v_min; scale = scale - stepSize) {
        var y = columnSize + (graph.yScale * count * stepSize);
        graph.context.fillText(scale + unit, margin, y + margin);
        graph.context.moveTo(rowSize, y + margin)
        graph.context.lineTo(graph.canvas.width, y + margin)
        count++;
    }
    graph.context.stroke();

    graph.context.translate(rowSize, graph.canvas.height + v_min * graph.yScale);
    graph.context.scale(1, -1 * graph.yScale);

    return graph;
}

function plotGraph(canvasId, dataSet, range_max, graphColor, unit) {
    var sections = 12;
    var graph = setScale(canvasId, range_max, sections, unit);

    l = dataSet.length;
    if (l > sections) {
        l = sections;
    }

    graph.context.fillStyle = graphColor;
    graph.context.beginPath();
    graph.context.moveTo(0, 0);

    for (i = 0; i < l; i++) {
        var this_val = dataSet[i];
        if (this_val > range_max) {
            this_val = range_max;
        } else if (this_val < 0) {
            this_val = 0;
        }
        graph.context.lineTo(i * graph.xScale, this_val);
        graph.context.lineTo((i + 1) * graph.xScale, this_val);
    }

    graph.context.lineTo((l) * graph.xScale, 0);
    graph.context.closePath();
    graph.context.fill();
}