import QtQuick 2.2
import QtQuick.Dialogs 1.1
import mplane 1.0
import "controls"

Page {
    id: root
    title: qsTr("Login")

    height: column.height


    property bool isWizard: false
    property bool loginMode: true
    property string buttonTitle: qsTr("Login")

    activity: client.loginController.status == Controller.Running

    function login() {
        if ( !mailField.acceptableInput ) {
            errorLabel.text = qsTr("Email is not valid");
            return;
        }

        if ( passwordField.text.length < 6 ) {
            errorLabel.text = qsTr("Password must be 6 characters or more");
            return;
        }

        if ( !root.loginMode && passwordField.text != passwordField2.text ) {
            errorLabel.text = qsTr("Passwords don't match");
            return;
        }

        if (root.loginMode) {
            client.settings.userId = mailField.text;
            client.settings.password = passwordField.text;
            client.loginController.login();
        } else {
            client.loginController.registration(mailField.text, passwordField.text);
        }
    }

    Connections {
        id: requester

        target: client.loginController

        onError: {
            console.log("Login error: " + target.errorString())
            errorLabel.text = target.errorString();
        }

        onFinished: {
            console.log("Login finished")

            if ( target.registeredDevice ) {
                console.log("Device is already registered: " + root.isWizard);

                if ( root.isWizard )
                    app.menuPage();
                else
                    pageStack.pop();
            }
            else {
                var properties = {
                    item: Qt.resolvedUrl("DeviceRegistration.qml"),
                    replace: true,
                    properties: {
                        isWizard: root.isWizard
                    }
                }

                console.log("Pushing device registration");

                if ( root.isWizard ) {
                    pageStack.insert(0, properties);
                    pageStack.pop(null);
                } else
                    pageStack.push(properties);
            }
        }
    }

    Column {
        id: column
        anchors.horizontalCenter: parent.horizontalCenter
//        spacing: units.gu(10)

        Column {
            anchors.horizontalCenter: parent.horizontalCenter

            TextField {
                id: mailField
                text: root.loginMode ? client.settings.userId : ""
                validator: RegExpValidator {
                    regExp: /.+@.+\..+/
                }
                onAccepted: passwordField.forceActiveFocus()
            }
        }

        Column {
            anchors.horizontalCenter: parent.horizontalCenter

            TextField {
                id: passwordField
                text: root.loginMode ? client.settings.password : ""
                echoMode: TextInput.Password
                onAccepted: {
                    if (root.loginMode)
                        root.login();
                    else
                        passwordField2.forceActiveFocus();
                }
            }
        }

        Column {
            visible: !root.loginMode
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                text: qsTr("Confirm password")
                font.pixelSize: units.gu(40)
            }

            TextField {
                id: passwordField2
                echoMode: TextInput.Password
                onAccepted: root.login()
            }
        }

        Label {
            id: errorLabel
            color: "blue"
            font.pixelSize: units.gu(25)
            x: text.length > 0 ? parent.width/2-errorLabel.width/2 : parent.width

            Behavior on x {
                NumberAnimation {
                    easing.type: Easing.InBack
                    duration: 100
                }
            }
        }

        Button {
            text: root.buttonTitle
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: root.login()
        }
    }
}
