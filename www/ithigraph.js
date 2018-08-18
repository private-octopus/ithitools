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