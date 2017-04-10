module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "text",
    "defaultValue": "To switch watchface press middle key and then back key after settings."
  },
  {
      "type": "toggle",
      "messageKey": "DIGITAL",
      "defaultValue": "true",
      "label": "Digital Watchface (on/off)"
  },
  {
    "type": "text",
    "defaultValue": "Routes."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Blue route"
      },
      {
        "type": "input",
        "messageKey": "BLUEFROM",
        "defaultValue": "3012430",
        "label": "Blue from"
      },
      {
        "type": "input",
        "messageKey": "BLUETO",
        "defaultValue": "3010011",
        "label": "Blue to"
      }
    ]
  },
   {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Red route"
      },
      {
        "type": "input",
        "messageKey": "REDFROM",
        "defaultValue": "3010011",
        "label": "Red from"
      },
      {
        "type": "input",
        "messageKey": "REDTO",
        "defaultValue": "3012430",
        "label": "Red to"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];