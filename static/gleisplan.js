


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





