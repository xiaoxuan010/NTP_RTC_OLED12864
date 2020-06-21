const char PAGE_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>小轩010的ESP8266</title>
    </head>

    <body>
        <script>
            function hideTimeInput(){
                document.getElementById('timeInput').style.display='none';
                document.getElementById('timeInputSubmit').style.display='none';
            }
            
            function showTimeInput(){
                document.getElementById('timeInput').style.display='block';
                document.getElementById('timeInputSubmit').style.display='block';
            }

            function submitAlarmForm(){
                document.getElementById("alarmForm").submit();
            }

            function submitDisplayForm(){
                document.getElementById("displayForm").submit();
            }
        </script>

        <h3>ESP8266 设置</h3>
        <h4>闹钟</h4>
        <form id='alarmForm' method="POST" action="/settings/alarm" enctype="multipart/form-data">
            <p>闹钟：
                <input onclick='showTimeInput()' type="radio" name="isAlarmOn" value="true" />开
                <input onclick='submitAlarmForm()' type="radio" name="isAlarmOn" value="false" />关
            </p>
            <p id="timeInput" style='display:none'>时间：<input type="time" name="alarmTime" \></p>
            <input id="timeInputSubmit" style='display:none' type="submit" value="确定">
        </form>
        <br/>
        <h4>显示器</h4>
            <form id='displayForm' method="POST" action="/settings/display" enctype="multipart/form-data">
                <p>显示屏：
                    <input onclick='submitDisplayForm()' type="radio"  name="isSavePower" value="0" />开
                    <input onclick='submitDisplayForm()' type="radio"  name="isSavePower" value="1" />关
                </p>
            </form>
        <br/><br/>
        <h3>ESP8266 固件上传</h3>
        <form method="POST" action="/update" enctype="multipart/form-data">
            <p>请选择固件文件：
                <input type="file" accept=".bin" name="update"  />
                <input type="submit" value='上传'>
            </p>
        </form>
    </body>
</html>
)=====";
