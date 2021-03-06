import QtQuick 2.1
import mplane 1.0
import "../controls"
import "../"

Base {
    id: root

    title: qsTr("Ping")

    measurement: "ping"
    properties: [
        {
            "label": "Host",
            "type": "TextField",
            "text": "measure-it.de"
        },

        {
            "label": "Count",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 1,
            "maximumValue": 255,
            "value": 3
        },

        {
            "label": "Interval",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 25,
            "maximumValue": 10000,
            "value": 1000,
            "format": qsTr("%1 ms")
        },

        {
            "label": "Timeout",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 25,
            "maximumValue": 20000,
            "value": 1000,
            "format": qsTr("%1 ms")
        },

        {
            "label": "Time to live",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 1,
            "maximumValue": 255,
            "value": 54
        },

        {
            "label": "Destination Port",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 1025,
            "maximumValue": 65535,
            "value": 33434
        },

        {
            "label": "Source Port",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 1025,
            "maximumValue": 65535,
            "value": 33434
        },

        {
            "label": "Payload",
            "type": "Slider",
            "stepSize": 1,
            "minimumValue": 1,
            "maximumValue": 1450,
            "value": 74,
            "format": qsTr("%1 bytes")
        },

        {
            "label": "Type",
            "type": "ComboBox",
            "model": ["System", "Udp", "Tcp"]
        }

    ]
}
