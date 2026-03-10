// ─────────────────────────────────────────────
//  THIS JAVA CODE IS USED TO COLLECT THE DATA FROM ESP32 VIS HTTP.
//  I DON'T HAVE ANY WRITE TO THIS CODE.
//  I HAVE TAKEN THIS CODE FROM YOUTUBE VIDEO CREATOR "ASLAM HOSSIAN"
// ─────────────────────────────────────────────


var ss = SpreadsheetApp.openById('Enter Sheet ID');
var sheet = ss.getSheetByName('Sheet1');
var timezone = "Enter Time Zone of Your Country";


function doGet(e){
  Logger.log( JSON.stringify(e) );
  if (e.parameter == 'undefined') {
    return ContentService.createTextOutput("Received data is undefined");
  }
  var Curr_Date = new Date();
  var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss');
  var name = stripQuotes(e.parameters.name);
  var nextRow = sheet.getLastRow() + 1;
  sheet.getRange("A" + nextRow).setValue(name);
  sheet.getRange("B" + nextRow).setValue(Curr_Date);
  sheet.getRange("C" + nextRow).setValue(Curr_Time);

  return ContentService.createTextOutput("Card holder name is stored in column A");
}


function stripQuotes( value ) {
  return value.toString().replace(/^["']|['"]$/g, "");
}

function doPost(e) {
  var val = e.parameter.value;
  
  if (e.parameter.value !== undefined){
    var range = sheet.getRange('A2');
    range.setValue(val);
  }
}
