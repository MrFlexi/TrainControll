
var gv_tid = 1;
var grid = 60;
var grid_width = 1000;
var grid_hight = 500;

function makeCircleW(left, top, line1, line2) {
    var c = new fabric.Circle({
        left: left,
        top: top,
        strokeWidth: 3,
        radius: 6,
        fill: '#fff',
        stroke: '#666'
    });
    c.hasControls = c.hasBorders = false;

    c.line1 = line1;
    c.line2 = line2;
    return c;
};

function displayGrid() {
    // create grid
    // left to right lines	
    for (var i = 0; i < (grid_width / grid); i++) {
        gl_canvas.add(new fabric.Line([i * grid, 0, i * grid, grid_hight], {
            stroke: '#ccc',
            selectable: false
        }));
    }

    gl_canvas.add(new fabric.Line([grid_width - 1, 0, grid_width - 1, grid_hight], {
        stroke: '#cccc',
        selectable: false
    }));

    // top to bottom lines
    for (var i = 0; i < (grid_hight / grid); i++) {
        gl_canvas.add(new fabric.Line([0, i * grid, grid_width, i * grid], {
            stroke: '#ccc',
            selectable: false
        }))
    }

    gl_canvas.add(new fabric.Line([0, grid_hight - 1, grid_width - 1, grid_hight - 1], {
        stroke: '#ccc',
        selectable: false
    }));


    // big separator line

    i = 3;
    gl_canvas.add(new fabric.Line([i * grid, 0, i * grid, grid_hight], {
        stroke: '#346187',
        selectable: false
    }));



}


// SAP Icon Background dark blue '#346187'

function makeTile(x, y) {
    var rect = new fabric.Rect({
        left: x,
        top: y,
        originX: 'left',
        originY: 'top',
        fill: '#346187',
        width: grid,
        height: grid,
        strokeWidth: 1,
        stroke: "#346187",
        rx: 10,
        ry: 10,
        angle: 0,
        scaleX: 1,
        scaleY: 1,
        opacity: 0.7,
        hasControls: false,
        centeredRotation: true
    })
    return rect;
}

function makeImage(x, y) {

    return fabric.loadSVGFromURL('/static/images/switch.svg', function (objects, options) {
        var obj2 = fabric.util.groupSVGElements(objects, options);
        return obj2
    });


}

function makeLineW(coords) {
    return new fabric.Line(coords, {
        fill: 'white',
        stroke: 'white',
        strokeWidth: 4,
        selectable: false,
        evented: false,
    });
};


function createW(id, x, y, aus) {
    position_x = x;
    position_y = y;
    x = x * grid;
    y = y * grid;
    var offset = grid / 2;
    var m = 2;   // Margin
    var text = new fabric.Text(String(id), { fontSize: 12, left: x + 5, top: y + 5 });

    var linew1 = makeLineW([x + m, y + offset, x + grid - m, y + offset]);
    if (aus == 'left') {
        var linew2 = makeLineW([x + m, y + offset, x + offset, y + m]);
    }

    if (aus == 'right') {
        var linew2 = makeLineW([x + m, y + offset, x + offset, y + grid - m]);
    }

    var group = new fabric.Group([makeTile(x, y), linew2, linew1, text], {
        id: id,
        dir: 0,
        angle: 0,
        left: x,
        top: y,
        line1: linew1,
        line2: linew2,
        centeredRotation: true,
        position_x: position_x,
        position_y: position_y,
        type: 'switch'
    });
    return group;
}


function ausWl(id, x, y) {
    position_x = x;
    position_y = y;
    x = x * grid;
    y = y * grid;
    var offset = grid / 2;

    var linew1 = makeLineW([x + offset, y, x + grid, y + offset]);
    var group = new fabric.Group([makeTile(x, y), linew1], {
        id: id,
        dir: 0,
        angle: 0,
        left: x,
        top: y,
        position_x: position_x,
        position_y: position_y,
        type: 'ausW1'
    });
    return group;
}

function track_g(id, x, y) {
    x = x * grid;
    y = y * grid;
    var offset = grid / 2;

    var linew1 = makeLineW([x, y + offset, x + grid, y + offset]);
    var group = new fabric.Group([makeTile(x, y), linew1], {
        id: id,
        dir: 0,
        angle: 0,
        left: x,
        top: y,
        position_x: position_x,
        position_y: position_y,
        type: 'track_g'
    })

    return group;
}


function toggle(o) {

    var a = o.target.getObjects();
    var c = o.target;
    var dir = o.target.dir;
    if (dir == 0) {
        c.line1 && c.line1.set({ 'stroke': 'gray', 'strokeWidth': 4 });
        c.line2 && c.line2.set({ 'stroke': 'white', 'strokeWidth': 4 });
        c.line2 && c.line2.bringToFront();
        dir = 1;
    }
    else {
        c.line2 && c.line2.set({ 'stroke': 'gray', 'strokeWidth': 4 })
        c.line1 && c.line1.set({ 'stroke': 'white', 'strokeWidth': 4 });
        c.line1 && c.line1.bringToFront();
        dir = 0;
    }
    o.target && o.target.set({ 'dir': dir })
}

function setFabricMode(state) {
    if (state) {
        gl_canvas.forEachObject(function (o) {
            o.hasControls = true;
            o.lockMovementX = false;
            o.lockMovementY = false;
            o.hasBorders = true;
        });
    }
    else {
        gl_canvas.forEachObject(function (o) {
            o.hasControls = false;
            o.lockMovementX = true;
            o.lockMovementY = true;
            o.hasBorders = false;
        });
    }
}

function FabricGetAllElements() {
    gl_canvas.forEachObject(function (o) {
        console.log(o.position_x + " " + o.position_y + " " + o.type + " " + o.wid)
    });
}


function resizeCanvas() {
    const outerCanvasContainer = $('.fabric-canvas-wrapper')[0];
    const ratio = gl_canvas.getWidth() / gl_canvas.getHeight();
    const containerWidth = outerCanvasContainer.clientWidth;
    const containerHeight = outerCanvasContainer.clientHeight;
    const scale = containerWidth / gl_canvas.getWidth();
    const zoom = gl_canvas.getZoom() * scale;

    gl_canvas.setDimensions({ width: containerWidth, height: containerWidth / ratio });
    gl_canvas.setViewportTransform([zoom, 0, 0, zoom, 0, 0]);
}

function renderGleisplan(viewId) {
    var cv = viewId + "--__fabric--canvas";    //
    var canvas = new fabric.Canvas(cv, {
        fireRightClick: true,
        stopContextMenu: true,
        width: 1000,
        height: 500
    });

    $(window).resize(resizeCanvas);

    gl_canvas = canvas;

    //-------------------------------------------------------------------------   
    // Register events
    //-------------------------------------------------------------------------   

    canvas.on('object:rotated', function (e) {

        var a = Math.round(e.target.angle / 90) * 90;
        e.target.animate('angle', a, {
            onChange: canvas.renderAll.bind(canvas),
            duration: 100,
            centeredRotation: true
        })
    });

    canvas.on('object:moving', function (e) {
        if (Math.round(e.target.left / grid * 1) % 1 == 0 &&
            Math.round(e.target.top / grid * 1) % 1 == 0) {
            e.target.set({
                left: Math.round(e.target.left / grid) * grid,
                top: Math.round(e.target.top / grid) * grid
            }).setCoords();
        }

    });

    canvas.on('mouse:down', function (o) {

        if (o.button === 3) {
            console.log("right click");
        }

        if (o.target) {
            if (o.target.type == 'group') {
                toggle(o);
                socket.emit('track_changed', { id: o.target.id, dir: o.target.dir });
            }

            if (o.target.type == 'circle') {
                o.target.line2 && o.target.line2.set({ 'stroke': 'green' });
            }
            console.log('an object was clicked! ', o.target.type, o.target.wid);
            canvas.renderAll();
        }
    })

    fabric.Object.prototype.originX = 'left';
    fabric.Object.prototype.originY = 'top';

    displayGrid();
    var w1 = createW(1, 0, 0, "left");
    canvas.add(w1);

    var w1 = createW(2, 1, 0, "right");
    canvas.add(w1);

    var w1 = createW(3, 2, 0, "left");
    canvas.add(w1);

    var w1 = createW(4, 0, 1, "right");
    canvas.add(w1);

    var w1 = createW(5, 1, 1, "right");
    canvas.add(w1);

    var w1 = createW(6, 2, 1, "right");
    canvas.add(w1);

    var w1 = createW(7, 0, 2, "left");
    canvas.add(w1);

    var w1 = createW(8, 1, 2, "left");
    canvas.add(w1);

    var w1 = createW(9, 2, 2, "right");
    canvas.add(w1);


    gv_tid = 20;

    var w1 = ausWl(gv_tid, 0, 4);
    canvas.add(w1);
    gv_tid++;

    var w1 = ausWl(gv_tid, 1, 4);
    canvas.add(w1);
    gv_tid++;

    var w1 = ausWl(gv_tid, 2, 4);
    canvas.add(w1);
    gv_tid++;

    var w1 = track_g(gv_tid, 0, 5);
    canvas.add(w1);
    gv_tid++;

    var w1 = track_g(gv_tid, 1, 5);
    canvas.add(w1);
    gv_tid++;

}



