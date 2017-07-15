char petDoorHTML[] = "<h3>Pet Door</h3>"
                "<form method='get' action='settingpetdoor'>"
                "<label>Open Time: </label><input type='time' name='opentime' length=4><br>"
                "<label>Close Time: </label><input type='time'name='closetime' length=4><br>"
                "<label>Up Seconds: </label><input name='upsec' length=4><br>"
                "<label>Down Seconds: </label><input name='downsec' length=4><br>"
                "<br><input name='pass' length=4>"
                "<input type='submit'><br>"
                "</form>"
                "end";


//var unixTimestamp = Date.now(); // in milliseconds
//Math.floor((new Date()).getTime() / 1000));   
char setTimeJS[] = "<script>"
                   ""
                   " var now = new Date(); "
                   " var dtField = document.getElementById('newtime');"
                   " dtField= now.getTime() - now.getTimezoneOffset() * 60000;"
                   " console.log('time'+dtField); "
                   "</script>";

char setTimeHTML[] = "<h3>Set Time</h3>"
                "<form method='get' action='settime'>"
                "<label id='clienttimelabel'>Client Time</label>"
                "<input type='datetime-local' id='newtime' name='newtime' length=32 >"
                "<br><input type='submit'><br>"
                "</form>"
                "end";




