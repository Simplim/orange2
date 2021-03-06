"""
Widget Tool Box
===============


A tool box with a tool grid for each category.

"""

import logging

from PyQt4.QtGui import (
    QAbstractButton, QSizePolicy, QAction, QApplication, QDrag, QPalette,
    QBrush, QIcon
)

from PyQt4.QtCore import (
    Qt, QObject, QModelIndex, QSize, QEvent, QMimeData, QByteArray,
    QDataStream, QIODevice
)

from PyQt4.QtCore import  pyqtSignal as Signal, pyqtProperty as Property

from ..gui.toolbox import ToolBox
from ..gui.toolgrid import ToolGrid
from ..gui.quickhelp import StatusTipPromoter
from ..gui.utils import create_gradient
from ..registry.qt import QtWidgetRegistry
from ..utils import qtcompat


log = logging.getLogger(__name__)


def iter_item(item):
    """
    Iterate over child items of a `QStandardItem`.
    """
    for i in range(item.rowCount()):
        yield item.child(i)


def iter_index(model, index):
    """
    Iterate over child indexes of a `QModelIndex` in a `model`.
    """
    for row in range(model.rowCount(index)):
        yield model.index(row, 0, index)


def qvariant_to_string(variant):
    if qtcompat.HAS_QVARIANT:
        return unicode(variant.toString())
    else:
        return unicode(variant)


def qvariant_to_icon(variant):
    if qtcompat.HAS_QVARIANT:
        value = variant.toPyObject()
    else:
        value = variant

    if isinstance(value, QIcon):
        return value
    else:
        return QIcon()


def qvariant_to_object(variant):
    if qtcompat.HAS_QVARIANT:
        value = variant.toPyObject()
    else:
        value = variant
    return value


def item_text(index):
    return qvariant_to_string(index.data(Qt.DisplayRole))


def item_icon(index):
    return qvariant_to_icon(index.data(Qt.DecorationRole))


def item_tooltip(index):
    return qvariant_to_string(index.data(Qt.DisplayRole))


class WidgetToolGrid(ToolGrid):
    """
    A Tool Grid with widget buttons. Populates the widget buttons
    from a item model. Also adds support for drag operations.

    """
    def __init__(self, *args, **kwargs):
        ToolGrid.__init__(self, *args, **kwargs)

        self.__model = None
        self.__rootIndex = None
        self.__actionRole = QtWidgetRegistry.WIDGET_ACTION_ROLE

        self.__dragListener = DragStartEventListener(self)
        self.__dragListener.dragStartOperationRequested.connect(
            self.__startDrag
        )
        self.__statusTipPromoter = StatusTipPromoter(self)

    def setModel(self, model, rootIndex=QModelIndex()):
        """
        Set a model (`QStandardItemModel`) for the tool grid. The
        widget actions are children of the rootIndex.

        .. warning:: The model should not be deleted before the
                     `WidgetToolGrid` instance.

        """
        if self.__model is not None:
            self.__model.rowsInserted.disconnect(self.__on_rowsInserted)
            self.__model.rowsRemoved.disconnect(self.__on_rowsRemoved)
            self.__model = None

        self.__model = model
        self.__rootIndex = rootIndex

        if self.__model is not None:
            self.__model.rowsInserted.connect(self.__on_rowsInserted)
            self.__model.rowsRemoved.connect(self.__on_rowsRemoved)

        self.__initFromModel(model, rootIndex)

    def model(self):
        """
        Return the model for the tool grid.
        """
        return self.__model

    def rootIndex(self):
        """
        Return the root index of the model.
        """
        return self.__rootIndex

    def setActionRole(self, role):
        """
        Set the action role. This is the model role containing a
        `QAction` instance.

        """
        if self.__actionRole != role:
            self.__actionRole = role
            if self.__model:
                self.__update()

    def actionRole(self):
        """
        Return the action role.
        """
        return self.__actionRole

    def actionEvent(self, event):
        if event.type() == QEvent.ActionAdded:
            # Creates and inserts the button instance.
            ToolGrid.actionEvent(self, event)

            button = self.buttonForAction(event.action())
            button.installEventFilter(self.__dragListener)
            button.installEventFilter(self.__statusTipPromoter)
            return
        elif event.type() == QEvent.ActionRemoved:
            button = self.buttonForAction(event.action())
            button.removeEventFilter(self.__dragListener)
            button.removeEventFilter(self.__statusTipPromoter)

            # Removes the button
            ToolGrid.actionEvent(self, event)
            return
        else:
            ToolGrid.actionEvent(self, event)

    def __initFromModel(self, model, rootIndex):
        """
        Initialize the grid from the model with rootIndex as the root.
        """
        for i, index in enumerate(iter_index(model, rootIndex)):
            self.__insertItem(i, index)

    def __insertItem(self, index, item):
        """
        Insert a widget action from `item` (`QModelIndex`) at `index`.
        """
        value = item.data(self.__actionRole)
        if value.isValid():
            action = value.toPyObject()
        else:
            action = QAction(item_text(item), self)
            action.setIcon(item_icon(item))
            action.setToolTip(item_tooltip(item))

        self.insertAction(index, action)

    def __update(self):
        self.clear()
        self.__initFromModel(self.__model, self.__rootIndex)

    def __on_rowsInserted(self, parent, start, end):
        """
        Insert items from range start:end into the grid.
        """
        if parent == self.__rootIndex:
            for i in range(start, end + 1):
                item = self.__rootIndex.child(i, 0)
                self.__insertItem(i, item)

    def __on_rowsRemoved(self, parent, start, end):
        """
        Remove items from range start:end from the grid.
        """
        if parent == self.__rootIndex:
            for i in reversed(range(start - 1, end)):
                action = self.actions()[i]
                self.removeAction(action)

    def __startDrag(self, button):
        """
        Start a drag from button
        """
        action = button.defaultAction()
        desc = action.data().toPyObject()  # Widget Description
        icon = action.icon()
        drag_data = QMimeData()
        drag_data.setData(
            "application/vnv.orange-canvas.registry.qualified-name",
            desc.qualified_name
        )
        drag = QDrag(button)
        drag.setPixmap(icon.pixmap(self.iconSize()))
        drag.setMimeData(drag_data)
        drag.exec_(Qt.CopyAction)


class DragStartEventListener(QObject):
    """
    An event filter object that can be used to detect drag start
    operation on buttons which otherwise do not support it.

    """
    dragStartOperationRequested = Signal(QAbstractButton)
    """A drag operation started on a button."""

    def __init__(self, parent=None, **kwargs):
        QObject.__init__(self, parent, **kwargs)
        self.button = None
        self.buttonDownObj = None
        self.buttonDownPos = None

    def eventFilter(self, obj, event):
        if event.type() == QEvent.MouseButtonPress:
            self.buttonDownPos = event.pos()
            self.buttonDownObj = obj
            self.button = event.button()

        elif event.type() == QEvent.MouseMove and obj is self.buttonDownObj:
            if (self.buttonDownPos - event.pos()).manhattanLength() > \
                    QApplication.startDragDistance() and \
                    not self.buttonDownObj.hitButton(event.pos()):
                # Process the widget's mouse event, before starting the
                # drag operation, so the widget can update its state.
                obj.mouseMoveEvent(event)
                self.dragStartOperationRequested.emit(obj)

                obj.setDown(False)

                self.button = None
                self.buttonDownPos = None
                self.buttonDownObj = None
                return True  # Already handled

        return QObject.eventFilter(self, obj, event)


class WidgetToolBox(ToolBox):
    """
    `WidgetToolBox` widget shows a tool box containing button grids of
    actions for a :class:`QtWidgetRegistry` item model.

    """

    triggered = Signal(QAction)
    hovered = Signal(QAction)

    def __init__(self, parent=None):
        ToolBox.__init__(self, parent)
        self.__model = None
        self.__iconSize = QSize(25, 25)
        self.__buttonSize = QSize(50, 50)
        self.setSizePolicy(QSizePolicy.Fixed,
                           QSizePolicy.Expanding)

    def setIconSize(self, size):
        """
        Set the widget icon size (icons in the button grid).
        """
        self.__iconSize = size
        for widget in  map(self.widget, range(self.count())):
            widget.setIconSize(size)

    def iconSize(self):
        """
        Return the widget buttons icon size.
        """
        return self.__iconSize

    iconSize_ = Property(QSize, fget=iconSize, fset=setIconSize,
                         designable=True)

    def setButtonSize(self, size):
        """
        Set fixed widget button size.
        """
        self.__buttonSize = size
        for widget in map(self.widget, range(self.count())):
            widget.setButtonSize(size)

    def buttonSize(self):
        """Return the widget button size
        """
        return self.__buttonSize

    buttonSize_ = Property(QSize, fget=buttonSize, fset=setButtonSize,
                           designable=True)

    def saveState(self):
        """
        Return the toolbox state (as a `QByteArray`).

        .. note:: Individual tabs are stored by their action's text.

        """
        version = 2

        actions = map(self.tabAction, range(self.count()))
        expanded = [action for action in actions if action.isChecked()]
        expanded = [action.text() for action in expanded]

        byte_array = QByteArray()
        stream = QDataStream(byte_array, QIODevice.WriteOnly)
        stream.writeInt(version)
        stream.writeQStringList(expanded)

        return byte_array

    def restoreState(self, state):
        """
        Restore the toolbox from a :class:`QByteArray` `state`.

        .. note:: The toolbox should already be populated for the state
                  changes to take effect.

        """
        # In version 1 of saved state the state was saved in
        # a simple dict repr string.
        if isinstance(state, QByteArray):
            stream = QDataStream(state, QIODevice.ReadOnly)
            version = stream.readInt()
            if version == 2:
                expanded = stream.readQStringList()
                for action in map(self.tabAction, range(self.count())):
                    if (action.text() in expanded) != action.isChecked():
                        action.trigger()

                return True
        return False

    def setModel(self, model):
        """
        Set the widget registry model (:class:`QStandardItemModel`) for
        this toolbox.

        """
        if self.__model is not None:
            self.__model.dataChanged.disconnect(self.__on_dataChanged)
            self.__model.rowsInserted.disconnect(self.__on_rowsInserted)
            self.__model.rowsRemoved.disconnect(self.__on_rowsRemoved)

        self.__model = model
        if self.__model is not None:
            self.__model.dataChanged.connect(self.__on_dataChanged)
            self.__model.rowsInserted.connect(self.__on_rowsInserted)
            self.__model.rowsRemoved.connect(self.__on_rowsRemoved)

        self.__initFromModel(self.__model)

    def __initFromModel(self, model):
        for row in range(model.rowCount()):
            self.__insertItem(model.index(row, 0), self.count())

    def __insertItem(self, item, index):
        """
        Insert category item  (`QModelIndex`) at index.
        """
        grid = WidgetToolGrid()
        grid.setModel(item.model(), item)
        grid.actionTriggered.connect(self.triggered)
        grid.actionHovered.connect(self.hovered)

        grid.setIconSize(self.__iconSize)
        grid.setButtonSize(self.__buttonSize)

        text = item_text(item)
        icon = item_icon(item)
        tooltip = item_tooltip(item)

        # Set the 'tab-title' property to text.
        grid.setProperty("tab-title", text)
        grid.setObjectName("widgets-toolbox-grid")

        self.insertItem(index, grid, text, icon, tooltip)
        button = self.tabButton(index)

        # Set the 'highlight' color
        if item.data(Qt.BackgroundRole).isValid():
            brush = item.background()
        elif item.data(QtWidgetRegistry.BACKGROUND_ROLE).isValid():
            brush = item.data(QtWidgetRegistry.BACKGROUND_ROLE).toPyObject()
        else:
            brush = self.palette().brush(QPalette.Button)

        if not brush.gradient():
            gradient = create_gradient(brush.color())
            brush = QBrush(gradient)

        palette = button.palette()
        palette.setBrush(QPalette.Highlight, brush)
        button.setPalette(palette)

    def __on_dataChanged(self, topLeft, bottomRight):
        parent = topLeft.parent()
        if not parent.isValid():
            for row in range(topLeft.row(), bottomRight.row() + 1):
                item = topLeft.sibling(row, topLeft.column())
                button = self.tabButton(row)
                button.setIcon(item_icon(item))
                button.setText(item_text(item))
                button.setToolTip(item_tooltip(item))

    def __on_rowsInserted(self, parent, start, end):
        """
        Items have been inserted in the model.
        """
        # Only the top level items (categories) are handled here.
        if not parent.isValid():
            for i in range(start, end + 1):
                item = self.__model.index(i, 0)
                self.__insertItem(item, i)

    def __on_rowsRemoved(self, parent, start, end):
        """
        Rows have been removed from the model.
        """
        # Only the top level items (categories) are handled here.
        if not parent.isValid():
            for i in range(end, start - 1, -1):
                self.removeItem(i)
