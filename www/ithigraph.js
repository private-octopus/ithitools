// Common functions used in the ITHI web pages.

function setDateElement(dateValue) {
    dateElement = document.getElementById("dateHeading");
    dateElement.innerHTML = dateValue;
}

function setFormattedValElement(valElementId, x, format) {

    var elm;

    elm = document.getElementById(valElementId);

    switch (format) {
        case 0:
            if (x.toFixed(0)) {
                elm.innerHTML = "<b>" + x.toFixed(0) + "</b>";
            }
            else {
                elm.innerHTML = "<b>" + x + "</b>";
            }
            break;
        case 1:
            if (x.toFixed(2)) {
                elm.innerHTML = "<b>" + x.toFixed(2) + "%</b>";
            }
            else {
                elm.innerHTML = "<b>" + x + "</b>";
            }
            break;
        case 2:
            if (x.toFixed(2)) {
                elm.innerHTML = "<b>" + x.toFixed(2) + "</b>";
            }
            else {
                elm.innerHTML = "<b>" + x + "</b>";
            }
            break;
        default:
            elm.innerHTML = "<b>" + x + "</b>";
            break;
    }
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

function setValElementI(valElementId, x) {
    elm = document.getElementById(valElementId);
    if (x.toFixed(0)) {
        elm.innerHTML = "<b>" + x.toFixed(0) + "</b>";
    }
    else {
        elm.innerHTML = "<b>" + x + "</b>";
    }
}

function setStringElement(valElementId, x) {
    elm = document.getElementById(valElementId);
    elm.innerHTML = "<b>" + x + "</b>";
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
        average /= last + 1 - first;

        return average;
    }
}

function getAverageLastN(dataSet, N) {
    var i = 0;
    var last = 0;
    var first = 0;
    var average = 0;

    if (dataSet.length > 1) {
        last = dataSet.length - 2;
        if (dataSet.length > N + 1) {
            first = dataSet.length - (N + 1);
        }

        for (i = first; i <= last; i++) {
            average += dataSet[i];
        }
        average /= last + 1 - first;
    }

    return average;
}

function getMinElement(dataSet) {
    if (dataSet.length < 1) {
        return 0;
    } else {
        var i = 0;
        var minEl = dataSet[0];

        for (i = 1; i < dataSet.length; i++) {
            if (minEl > dataSet[i]) {
                minEl = dataSet[i];
            }
        }

        return minEl;
    }
}

function getMaxElement(dataSet) {
    if (dataSet.length < 1) {
        return 0;
    } else {
        var i = 0;
        var maxEl = dataSet[0];

        for (i = 1; i < dataSet.length; i++) {
            if (maxEl < dataSet[i]) {
                maxEl = dataSet[i];
            }
        }

        return maxEl;
    }
}

function getMaxRange(rawMax) {
    var i = 0;
    var t_max = 0.1;

    for (i = 0; i < 6; i++) {
        if (t_max > 0.99) {
            t_max = Math.round(t_max);
        }
        if (2.0 * t_max > rawMax) {
            return 2.0 * t_max;
        } else if (5.0 * t_max > rawMax) {
            return 5.0 * t_max;
        } else {
            t_max *= 10.0;

            if (t_max > rawMax) {
                return t_max;
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

function columnReminder(dataSet, targetTotal) {
    var i = 0;
    var j = 0;

    var reminder = new Array(dataSet[0].length);

    for (i = 0; i < reminder.length; i++) {
        reminder[i] = targetTotal;
    }

    for (j = 0; j < dataSet.length; j++) {
        var lineSet = dataSet[j];
        for (i = 0; i < reminder.length; i++) {
            reminder[i] -= lineSet[i];
        }
    }

    return reminder;
}

function summarizeNameSet(nameSet) {
    var i = 0;
    var j = 0;
    var example = nameSet[0];
    var summary = new Array(example.length);

    for (i = 0; i < summary.length; i++) {
        summary[i] = 0;
    }

    for (j = 0; j < nameSet.length; j++) {
        var lineSet = nameSet[j];
        var nameDataSet = lineSet[1];
        for (i = 0; i < summary.length; i++) {
            summary[i] += nameDataSet[i];
        }
    }

    return summary;
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
        alpha2 = alpha + 2 * dataSet[i] / 100;
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

function fillMetricTableNew(tableName, tableId, dataSet) {
    var i = 0;

    var tableElem = document.getElementById(tableId);
    var tableText = "<table class=\"metrics\"><tr><th>" + tableName + "</th>";

    tableText += "<th class=\"number\">Current Value</th>" +
           "<th class=\"number\">Past 3 months</th>" +
           "<th class=\"number\">Historic Low</th>" +
           "<th class=\"number\">Historic High</th></tr>\n";

    for (i = 0; i < dataSet.length; i++) {
        var j = 0;
        var lineSet = dataSet[i];
        var lineValueSet = lineSet[1];
        var lineValues = [
            getLastElement(lineSet[1]),
            getAverageLastN(lineSet[1], 3),
            getMinElement(lineSet[1]),
            getMaxElement(lineSet[1])];

        tableText += "<tr><td>" + lineSet[0] + "</td>";
        for (j = 0; j < 4; j++) {
            tableText += "<td class=\"number\">" + lineValues[j].toFixed(3) + "%</td>";
        }
        tableText += "</tr>\n";
    }
    tableElem.innerHTML += "</table>\n";

    tableElem.innerHTML = tableText;
}

function fillEdnsDoQname(rowNames, vEdns, vDo, vQname) {
    var i = 0;
    var tableElem = document.getElementById("tableEdnsDoQname");
    var tableText = "<table  class=\"metrics\">";
    var current = 0;
    var average = 0;

    tableText += "<tr><th colspan=2>Metric</th>";
    tableText += "<th class=\"number\">Current Value</th>";
    tableText += "<th class=\"number\">Average Value</th></tr>\n";

    // Set the Edns basic row 
    var vEdns1 = vEdns[0];
    var vEdns2 = vEdns[1];
    current = getLastElement(vEdns1);
    average = getAverageElement(vEdns1);

    tableText += "<tr><td>" + rowNames[0] + ".1</td><td>%resolvers using Extended DNS (EDNS) </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td></tr>\n";

    // Set the per option rows 
    if (vEdns2.length > 0) {
        tableText += "<tr><td rowspan=" + vEdns2.length + ">" + rowNames[0] + ".2</td>";

        for (i = 0; i < vEdns2.length; i++) {
            var j = 0;
            var lineSet = vEdns2[i];

            if (i > 0) {
                tableText += "<tr>";
            }

            tableText += "<td> %resolvers using " + lineSet[0] + "</td>";

            for (j = 1; j < 3 && j < lineSet.length; j++) {
                tableText += "<td class=\"number\">" + lineSet[j].toFixed(3) + "%</td>";
            }
            tableText += "</tr>\n";
        }
    }

    // DO line 
    current = getLastElement(vDo);
    average = getAverageElement(vDo);

    tableText += "<tr><td>" + rowNames[1] + "</td><td>%resolvers setting DNSSEC OK (DO) flag </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td></tr>\n";



    // QName line 
    current = getLastElement(vQname);
    average = getAverageElement(vQname);

    tableText += "<tr><td>" + rowNames[1] + "</td><td>%resolvers using QName minimization </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td></tr>\n";


    tableText += "</table>\n";
    tableElem.innerHTML = tableText;
}

function fillEdnsDoQnameNew(rowNames, vEdns, vDo, vQname) {
    var i = 0;
    var tableElem = document.getElementById("tableEdnsDoQname");
    var tableText = "<table  class=\"metrics\">";
    var current = 0;
    var average = 0;
    var vMin = 0;
    var vMax = 0;

    tableText += "<tr><th colspan=2>Metric</th>";
    tableText += "<th class=\"number\">Current Value</th>";
    tableText += "<th class=\"number\">Average Value</th>";
    tableText += "<th class=\"number\">Historic Low</th>";
    tableText += "<th class=\"number\">Historic High</th></tr>\n";

    // Set the Edns basic row 
    var vEdns1 = vEdns[0];
    var vEdns2 = vEdns[1];
    current = getLastElement(vEdns1);
    average = getAverageLastN(vEdns1, 3);
    vMin = getMinElement(vEdns1);
    vMax = getMaxElement(vEdns1);


    tableText += "<tr><td>" + rowNames[0] + ".1</td><td>%resolvers using Extended DNS (EDNS) </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td>\n";
    tableText += "<td class=\"number\">" + vMin.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + vMax.toFixed(3) + "%</td></tr>\n";

    // Set the per option rows 
    if (vEdns2.length > 0) {
        tableText += "<tr><td rowspan=" + vEdns2.length + ">" + rowNames[0] + ".2</td>";

        for (i = 0; i < vEdns2.length; i++) {
            var j = 0;
            var lineSet = vEdns2[i];

            if (i > 0) {
                tableText += "<tr>";
            }

            tableText += "<td> %resolvers using " + lineSet[0] + "</td>";

            current = getLastElement(lineSet[1]);
            average = getAverageLastN(lineSet[1], 3);
            vMin = getMinElement(lineSet[1]);
            vMax = getMaxElement(lineSet[1]);

            tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
            tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td>\n";
            tableText += "<td class=\"number\">" + vMin.toFixed(3) + "%</td>";
            tableText += "<td class=\"number\">" + vMax.toFixed(3) + "%</td></tr>\n";

            tableText += "</tr>\n";
        }
    }

    // DO line 
    current = getLastElement(vDo);
    average = getAverageLastN(vDo, 3);
    vMin = getMinElement(vDo);
    vMax = getMaxElement(vDo);


    tableText += "<tr><td>" + rowNames[1] + "</td><td>%resolvers setting DNSSEC OK (DO) flag </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td>\n";
    tableText += "<td class=\"number\">" + vMin.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + vMax.toFixed(3) + "%</td></tr>\n";



    // QName line 
    current = getLastElement(vQname);
    average = getAverageLastN(vQname, 3);
    vMin = getMinElement(vQname);
    vMax = getMaxElement(vQname);

    tableText += "<tr><td>" + rowNames[2] + "</td><td>%resolvers using QName minimization </td>";
    tableText += "<td class=\"number\">" + current.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + average.toFixed(3) + "%</td>\n";
    tableText += "<td class=\"number\">" + vMin.toFixed(3) + "%</td>";
    tableText += "<td class=\"number\">" + vMax.toFixed(3) + "%</td></tr>\n";


    tableText += "</table>\n";
    tableElem.innerHTML = tableText;
}


function setScale(canvasId, v_max, sections, unit) {
    var v_min = 0;
    var stepSize = v_max / 10;
    var columnSize = 50;
    var rowSize = 50;
    var margin = 10;
    var xAxis = [" ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
    var graph = new Object();
    var i = 0;
    graph.canvas = document.getElementById(canvasId);
    graph.context = graph.canvas.getContext("2d");
    graph.context.fillStyle = "#808080";
    graph.context.font = "20 pt Verdana";

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
        var y = columnSize + graph.yScale * count * stepSize;
        var fscale;
        if (v_max >= 10.0) {
            fscale = scale.toFixed(0);
        } else if (v_max >= 1.0) {
            fscale = scale.toFixed(1);
        } else {
            fscale = scale;
        }
        graph.context.fillText(fscale + unit, margin, y + margin);
        graph.context.moveTo(rowSize, y + margin);
        graph.context.lineTo(graph.canvas.width, y + margin);
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

    graph.context.lineTo(l * graph.xScale, 0);
    graph.context.closePath();
    graph.context.fill();
}



function fillValueAverageMinMax(pilot, dataSet, format) {
    setFormattedValElement(pilot[0], getLastElement(dataSet), format[0]);
    setFormattedValElement(pilot[1], getAverageLastN(dataSet, 3), format[1]);
    setFormattedValElement(pilot[2], getMinElement(dataSet), format[2]);
    setFormattedValElement(pilot[3], getMaxElement(dataSet), format[3]);
}

function setScaleNew(canvasId, v_max, sections, firstMonth, unit) {
    var v_min = 0;
    var stepSize = v_max / 10;
    var columnSize = 50;
    var rowSize = 50;
    var margin = 10;
    var graph = new Object();
    var i = 0;
    graph.canvas = document.getElementById(canvasId);
    graph.context = graph.canvas.getContext("2d");
    graph.context.fillStyle = "#555555";
    graph.context.font = "20 pt Verdana";

    graph.yScale = (graph.canvas.height - columnSize - margin) / (v_max - v_min);
    graph.xScale = (graph.canvas.width - rowSize) / (sections);

    graph.context.strokeStyle = "#555555"; // color of grid lines
    graph.context.beginPath();

    // print Parameters on X axis, and grid lines on the graph
    for (i = 1; i <= sections + 1; i++) {
        var x = rowSize + (i -1) * graph.xScale;
        if (i <= sections){
            graph.context.fillText(getMonthId( firstMonth + i - 1), x, columnSize - margin);
        }
        graph.context.moveTo(x, columnSize + margin);
        graph.context.lineTo(x, graph.canvas.height);
    }
    // print row header and draw horizontal grid lines
    var count = 0;
    var scale = 0;
    for (scale = v_max; scale >= v_min; scale = scale - stepSize) {
        var y = columnSize + graph.yScale * count * stepSize;
        var fscale;
        if (v_max >= 10.0) {
            fscale = scale.toFixed(0);
        } else if (v_max >= 1.0) {
            fscale = scale.toFixed(1);
        } else {
            fscale = scale;
        }
        graph.context.fillText(fscale + unit, margin, y + margin);
        graph.context.moveTo(rowSize, y + margin);
        graph.context.lineTo(graph.canvas.width, y + margin);
        count++;
    }
    graph.context.stroke();

    graph.context.translate(rowSize, graph.canvas.height + v_min * graph.yScale);
    graph.context.scale(1, -1 * graph.yScale);

    return graph;
}

function getMonthId(monthIndex){
    var monthList = ["   ", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

    while (monthIndex < 1) {
        monthIndex += 12;
    }

    while (monthIndex > 12) {
        monthIndex -= 12;
    }

    return monthList[monthIndex];
}

function getMonthIndex(lastMonth, delta){

    var monthIndex = lastMonth - delta;
    while (monthIndex < 1) {
        monthIndex += 12;
    }

    return monthIndex;
}

function getFirstMonthIndex(lastMonth, nbData){

    var monthIndex = lastMonth - nbData + 1;
    while (monthIndex < 1) {
        monthIndex += 12;
    }

    return monthIndex;
}

function plotGraphNew(canvasId, dataSet, range_max, firstMonth, graphColor, unit) {
    var sections = 12;

    l = dataSet.length;
    if (l >= sections) {
        sections = l+1;
    }

    var graph = setScaleNew(canvasId, range_max, sections, firstMonth, unit);

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

    graph.context.lineTo(l * graph.xScale, 0);
    graph.context.closePath();
    graph.context.fill();
}



function plotStackGraphNew(canvasId, dataSet1, dataSet2, rangeMax, firstMonth, colorSet, unit) {
    var sections = 12;
    var l = dataSet1.length;

    if (l >= sections) {
        sections = l + 1;
    }

    var graph = setScaleNew(canvasId, rangeMax, sections, firstMonth, unit);

    // Start with first data set
    var context = graph.context;
    var xScale = graph.xScale;
    var yScale = graph.yScale;

    context.fillStyle = colorSet[0];
    context.beginPath();
    context.moveTo(0, 0);

    for (i = 0; i < l; i++) {
        context.lineTo(i * xScale, dataSet1[i]);
        context.lineTo((i + 1) * xScale, dataSet1[i]);
    }

    context.lineTo(l * xScale, 0);
    context.closePath();
    context.fill();

    // Continue with second data set

    context.fillStyle = colorSet[1];
    context.beginPath();
    context.moveTo(0, dataSet1[0]);

    for (i = 0; i < l; i++) {
        context.lineTo(i * xScale, dataSet1[i] + dataSet2[i]);
        context.lineTo((i + 1) * xScale, dataSet1[i] + dataSet2[i]);
    }

    for (i = l - 1; i >= 0; i--) {
        context.lineTo((i + 1) * xScale, dataSet1[i]);
        context.lineTo(i * xScale, dataSet1[i]);
    }

    context.closePath();
    context.fill();

    // Continue with the complement, core

    context.fillStyle = colorSet[2];
    context.beginPath();

    context.moveTo(0, dataSet1[0] + dataSet2[0]);

    for (i = 1; i < l; i++) {
        context.lineTo(i * xScale, dataSet1[i] + dataSet2[i]);
        context.lineTo((i + 1) * xScale, dataSet1[i] + dataSet2[i]);
    }

    context.lineTo(l * xScale, rangeMax);
    context.lineTo(0, rangeMax);
    context.closePath();
    context.fill();
}

