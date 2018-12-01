function initnavL1(level) {
    document.getElementById("navMenu").innerHTML =
        '<div class="navbar">' +
        '<a href="' + setLevelString(level) + 'index.html">Home' + level + '</a>' +
        '<div class="dropdown">' +
        '<a href="' + setLevelString(level) + 'index.html">Metrics</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + setLevelString(level) + 'graph-m1.html">M1: Whois</a>' +
        '<a href="' + setLevelString(level) + 'graph-m2.html">M2: Abuses</a>' +
        '<a href="' + setLevelString(level) + 'graph-m3.html">M3: Root Servers</a>' +
        '<a href="' + setLevelString(level) + 'graph-m4.html">M4: Recursive Servers</a>' +
        '<a href="' + setLevelString(level) + 'graph-m5.html">M5: Recursive Resolver Integrity</a>' +
        '<a href="' + setLevelString(level) + 'graph-m6.html">M6: IANA</a>' +
        '<a href="' + setLevelString(level) + 'graph-m7.html">M7: DNSSEC</a>' +
        '<a href="' + setLevelString(level) + 'graph-m8.html">M8: Authoritative</a>' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="' + setLevelString(level) + 'participating.html">Participate</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + setLevelString(level) + 'participating.html">Participate</a>' +
        '<a href="' + setLevelString(level) + 'partners/partnersOnly.html">Partners Only</a>' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="' + setLevelString(level) + 'about.html">About</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + setLevelString(level) + 'about.html">About ITHI</a>' +
        '<a href="' + setLevelString(level) + 'about-m1.html">About M1</a>' +
        '<a href="' + setLevelString(level) + 'about-m2.html">About M2</a>' +
        '<a href="' + setLevelString(level) + 'bout-m3.html">About M3</a>' +
        '<a href="' + setLevelString(level) + 'about-m4.html">About M4</a>' +
        '<a href="' + setLevelString(level) + 'about-m5.html">About M5</a>' +
        '<a href="' + setLevelString(level) + 'about-m6.html">About M6</a>' +
        '<a href="' + setLevelString(level) + 'about-m7.html">About M7</a>' +
        '<a href="' + setLevelString(level) + 'about-m8.html">About M8</a>' +
        '</div>' +
        '</div>' +

        '</div>';
}

function setLevelString(level) {
    if ( level == 0) {
        return "./";
    } else {
        return "../";
    }
}