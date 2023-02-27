function initnav()
{
    return initnavL1(0);
}

function initnavL1(level) {
    var levelString;

    if (level === 0) {
        levelString = "./";
    } else {
        levelString = "../";
    }

    document.getElementById("navMenu").innerHTML =
        '<div class="lockup">' +
        '<img src="' + levelString + 'lockup.png" />' +
        '</div>' +
        '<div class="navbar">' +
        '<a href="' + levelString + 'index.html">Home</a>' +
        '<div class="dropdown">' +
        '<a href="' + levelString + 'metrics.html">Metrics</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + levelString + 'metrics.html">ITHI Metrics</a>' +
        '<a href="' + levelString + 'graph-m2.html">M2: Abuses</a>' +
        '<a href="' + levelString + 'graph-m3.html">M3: Root Servers</a>' +
        '<a href="' + levelString + 'graph-m4.html">M4: Recursive Servers</a>' +
        '<a href="' + levelString + 'graph-m5.html">M5: Recursive Resolver Integrity</a>' +
        '<a href="' + levelString + 'graph-m6.html">M6: IANA</a>' +
        '<a href="' + levelString + 'graph-m7.html">M7: DNSSEC</a>' +
        '<a href="' + levelString + 'graph-m8.html">M8: Authoritative</a>' +
        '<a href="' + levelString + 'graph-m9.html">M9: Concentration of Authoritative  Services</a>' +
        '<a href="' + levelString + 'graph-m10.html">M10: Concentration of DNS Resolver Services' +
        '<a href="' + levelString + 'graph-eai.html">EAI: Email Address Internationalization support in  mail servers' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="' + levelString + 'participating.html">Participate</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + levelString + 'participating.html">Participate</a>' +
        '<a href="' + levelString + 'partners/partnersOnly.html">Partners Only</a>' +
        '<a href="' + levelString + 'unlp/graph-m4.html">M4: Recursive Server Analysis for UNLP</a>' +
        '<a href="' + levelString + 'unlp/graph-m6.html">M6: DNS parameters at UNLP</a>' +
        '<a href="' + levelString + 'uccgh/graph-m4.html">M4: Recursive Server Analysis for UCC, Ghana</a>' +
        '<a href="' + levelString + 'uccgh/graph-m6.html">M6: DNS parameters at UCC, Ghana</a>' +
        '<a href="' + levelString + 'nawala/graph-m4.html">M4: Recursive Server Analysis for Nawala</a>' +
        '<a href="' + levelString + 'nawala/graph-m6.html">M6: DNS parameters at Nawala</a>' +
        '<a href="' + levelString + 'kaznic/graph-m8.html">M8: Authoritative Server analysis at KazNIC </a>' +
        '<a href="' + levelString + 'kaznic/graph-m6.html">M6: DNS Parameters at KazNIC</a>' +
        '<a href="' + levelString + 'twnic/graph-m8.html">M8: Authoritative Server Analysis for TWNIC</a>' +
        '<a href="' + levelString + 'twnic/graph-m6.html">M6: DNS Parameters at TWNIC</a>' +
        '<a href="' + levelString + 'upload/upload.html">Upload file to ITHI</a>' +
        '<a href="' + levelString + 'check/partnersCheck.html">Check Partner Status</a>' +
        '</div>' +
        '</div>' +

        '<div class="dropdown">' +
        '<a href="' + levelString + 'about.html">About</a>' +
        '<div class="dropdown-content">' +
        '<a href="' + levelString + 'about.html">About ITHI</a>' +
        '<a href="' + levelString + 'about-m1.html">About M1</a>' +
        '<a href="' + levelString + 'about-m2.html">About M2</a>' +
        '<a href="' + levelString + 'about-m3.html">About M3</a>' +
        '<a href="' + levelString + 'about-m4.html">About M4</a>' +
        '<a href="' + levelString + 'about-m5.html">About M5</a>' +
        '<a href="' + levelString + 'about-m6.html">About M6</a>' +
        '<a href="' + levelString + 'about-m7.html">About M7</a>' +
        '<a href="' + levelString + 'about-m8.html">About M8</a>' +
        '<a href="' + levelString + 'about-m9.html">About M9</a>' +
        '<a href="' + levelString + 'about-m10.html">About M10</a>' +
        '<a href="' + levelString + 'about-eai.html">About EAI</a>' +
        '<a href="https://ithi.research.icann.org/cgi-bin/awstats.pl?config=ithi.research.icann.org">Analytics</a>' +
        '</div>' +
        '</div>' +

        '</div>';

        var footer = document.createElement("div");

        footer.classList.add("footer");

        footer.innerHTML = '<span>Copyright &#169 2021, Internet Corporation for Assigned Names and Numbers</span>' +
        '<div>' +
        '<a href="https://www.icann.org/privacy/policy">Privacy Policy</a>' +
        '<a href="https://www.icann.org/privacy/tos">Terms of Service</a>' +
        '<a href="https://www.icann.org/privacy/cookies">Cookie Policy</a>' +
        '</div>';

        document.body.appendChild(footer);
}
