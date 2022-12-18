#pragma once

const char webControlPage[] PROGMEM =
    "<!DOCTYPE html>"
    "<html>"
    "<head><title>Laser level remote control</title></head>"
    "<body>"
    "<form action=\"api/move\" method=\"POST\" target=\"_blank\">"
    "<fieldset>"
    "<legend>Linear movement commands</legend>"
    "<input type=\"radio\" name=\"dist\" value=\"10\" checked>10"
    "<input type=\"radio\" name=\"dist\" value=\"100\">100"
    "<input type=\"radio\" name=\"dist\" value=\"500\">500"
    "<br />"
    "<button type=\"submit\" name=\"dir\" value=\"left\">Left</button>"
    "<button type=\"submit\" name=\"dir\" value=\"right\">Right</button>"
    "</fieldset>"
    "</form>"

    "<form action=\"api/stopmove\" method=\"POST\" target=\"_blank\">"
    "<button type=\"submit\">Stop movement</button>"
    "</form>"

    "<form action=\"api/turn\" method=\"POST\" target=\"_blank\">"
    "<fieldset>"
    "<legend>Rotation commands</legend>"
    "<input type=\"radio\" name=\"dist\" value=\"10\" checked>10"
    "<input type=\"radio\" name=\"dist\" value=\"100\">100"
    "<input type=\"radio\" name=\"dist\" value=\"500\">500"
    "<br />"
    "<button type=\"submit\" name=\"dir\" value=\"cw\">CW</button>"
    "<button type=\"submit\" name=\"dir\" value=\"ccw\">CCW</button>"
    "</fieldset>"
    "</form>"

    "<form action=\"api/stopturn\" method=\"POST\" target=\"_blank\">"
    "<button type=\"submit\">Stop rotation</button>"
    "</form>"

    "</body>"
    "</html>";
