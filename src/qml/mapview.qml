import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Rectangle {
    width: 800
    height: 600
	Item {
		id: flag
		property bool f: false
	}

    Plugin {
        id: osmPlugin
        name: "osm"
    }

    MapView {
        id: view
        anchors.fill: parent
        map.plugin: osmPlugin
        map.zoomLevel: 10
        focus: true

        Keys.onPressed: (event) => {
            if ((event.key === Qt.Key_Plus || event.key === Qt.Key_Equal) &&
                (event.modifiers & Qt.ControlModifier)) {
                view.map.zoomLevel++;
            } else if (event.key === Qt.Key_Minus &&
                       (event.modifiers & Qt.ControlModifier)) {
                view.map.zoomLevel--;
            } else if (event.key === Qt.Key_Left || event.key === Qt.Key_Right ||
                       event.key === Qt.Key_Up   || event.key === Qt.Key_Down) {
                var dx = 0;
                var dy = 0;
                switch (event.key) {
                case Qt.Key_Left: dx = view.map.width / 4; break;
                case Qt.Key_Right: dx = -view.map.width / 4; break;
                case Qt.Key_Up: dy = view.map.height / 4; break;
                case Qt.Key_Down: dy = -view.map.height / 4; break;
                }
                var mapCenterPoint = Qt.point(view.map.width / 2.0 - dx, view.map.height / 2.0 - dy);
                view.map.center = view.map.toCoordinate(mapCenterPoint);
            }
        }

        WheelHandler {
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: (event) => {
                const step = event.angleDelta.y > 0 ? 0.5 : -0.5
                view.map.zoomLevel = Math.max(1, Math.min(20, view.map.zoomLevel + step))
            }
        }
    }

    Connections {
        target: point
        function onDrawPoint(lat, lon) {
		  	if(!flag.f) {
				view.map.center = QtPositioning.coordinate(lat, lon)
				flag.f = true
			}

            const coord = QtPositioning.coordinate(lat, lon)

            const marker = Qt.createQmlObject(`
                import QtLocation
                import QtQuick
				import QtPositioning
                MapQuickItem {
                    anchorPoint.x: 8
                    anchorPoint.y: 8
                    coordinate: QtPositioning.coordinate(${lat}, ${lon})
                    sourceItem: Rectangle {
                        width: 12
                        height: 12
                        color: "red"
                        radius: 8
                        border.color: "black"
                        border.width: 1
                    }
                }
            `, view.map)

            view.map.addMapItem(marker)
        }
        }
}
