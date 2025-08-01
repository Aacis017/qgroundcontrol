/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.qmlmodels

import QGroundControl
import QGroundControl.Controls
import QGroundControl.Controllers
import QGroundControl.ScreenTools

AnalyzePage {
    id: logDownloadPage
    pageComponent: pageComponent
    pageDescription: qsTr("Log Download allows you to download binary log files from your vehicle. Click Refresh to get list of available logs.")

    Component {
        id: pageComponent

        RowLayout {
            width: availableWidth
            height: availableHeight

            QGCFlickable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: gridLayout.width
                contentHeight: gridLayout.height

                GridLayout {
                    id: gridLayout
                    rows: LogDownloadController.model.count + 1
                    columns: 5
                    flow: GridLayout.TopToBottom
                    columnSpacing: ScreenTools.defaultFontPixelWidth
                    rowSpacing: 0

                    QGCCheckBox {
                        id: headerCheckBox
                        enabled: false
                    }

                    Repeater {
                        model: LogDownloadController.model

                        QGCCheckBox {
                            Binding on checkState {
                                value: object.selected ? Qt.Checked : Qt.Unchecked
                            }

                            onClicked: object.selected = checked
                        }
                    }

                    QGCLabel { text: qsTr("Id") }

                    Repeater {
                        model: LogDownloadController.model

                        QGCLabel { text: object.id }
                    }

                    QGCLabel { text: qsTr("Date") }

                    Repeater {
                        model: LogDownloadController.model

                        QGCLabel {
                            text: {
                                if (!object.received) {
                                    return ""
                                }

                                if (object.time.getUTCFullYear() < 2010) {
                                    return qsTr("Date Unknown")
                                }

                                return object.time.toLocaleString(undefined)
                            }
                        }
                    }

                    QGCLabel { text: qsTr("Size") }

                    Repeater {
                        model: LogDownloadController.model

                        QGCLabel { text: object.sizeStr }
                    }

                    QGCLabel { text: qsTr("Status") }

                    Repeater {
                        model: LogDownloadController.model

                        QGCLabel { text: object.status }
                    }
                }
            }

            ColumnLayout {
                spacing: ScreenTools.defaultFontPixelWidth
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: false

                QGCButton {
                    Layout.fillWidth: true
                    enabled: !LogDownloadController.requestingList && !LogDownloadController.downloadingLogs
                    text: qsTr("Refresh")

                    onClicked: {
                        if (!QGroundControl.multiVehicleManager.activeVehicle || QGroundControl.multiVehicleManager.activeVehicle.isOfflineEditingVehicle) {
                            mainWindow.showMessageDialog(qsTr("Log Refresh"), qsTr("You must be connected to a vehicle in order to download logs."))
                            return
                        }

                        LogDownloadController.refresh()
                    }
                }

                QGCButton {
                    Layout.fillWidth: true
                    enabled: !LogDownloadController.requestingList && !LogDownloadController.downloadingLogs
                    text: qsTr("Download")

                    onClicked: {
                        var logsSelected = false
                        for (var i = 0; i < LogDownloadController.model.count; i++) {
                            if (LogDownloadController.model.get(i).selected) {
                                logsSelected = true
                                break
                            }
                        }

                        if (!logsSelected) {
                            mainWindow.showMessageDialog(qsTr("Log Download"), qsTr("You must select at least one log file to download."))
                            return
                        }

                        if (ScreenTools.isMobile) {
                            LogDownloadController.download()
                            return
                        }

                        fileDialog.title = qsTr("Select save directory")
                        fileDialog.folder = QGroundControl.settingsManager.appSettings.logSavePath
                        fileDialog.selectFolder = true
                        fileDialog.openForLoad()
                    }

                    QGCFileDialog {
                        id: fileDialog
                        onAcceptedForLoad: (file) => {
                            LogDownloadController.download(file)
                            close()
                        }
                    }
                }

                QGCButton {
                    Layout.fillWidth: true
                    enabled: !LogDownloadController.requestingList && !LogDownloadController.downloadingLogs && (LogDownloadController.model.count > 0)
                    text: qsTr("Erase All")
                    onClicked: mainWindow.showMessageDialog(
                        qsTr("Delete All Log Files"),
                        qsTr("All log files will be erased permanently. Is this really what you want?"),
                        Dialog.Yes | Dialog.No,
                        function() { LogDownloadController.eraseAll() }
                    )
                }

                QGCButton {
                    Layout.fillWidth: true
                    text: qsTr("Cancel")
                    enabled: LogDownloadController.requestingList || LogDownloadController.downloadingLogs
                    onClicked: LogDownloadController.cancel()
                }
            }
        }
    }
}
