window.themes = [
  {
    accent: [0, 220, 0],
    ghost: [0, 0, 200],
    food: [8, 66, 166],
    background: [10, 10, 40]
  },

  {
    accent: [245, 227, 66],
    ghost: [237, 90, 45],
    food: [191, 55, 2],
    background: [10, 10, 40]
  }
]

function clamp(value, low, high) {
  return max(min(value, high), low);
}

function weightedAvg(left, right, weight) {
  return (left * (1 - weight)) + (right * weight);
}

function distance(x1, y1, x2, y2) {
  return Math.sqrt(Math.pow(x2 - x1, 2) + Math.pow(y2 - y1, 2));
}

function angleTo(x1, y1, x2, y2) {
  return Math.atan2(y2 - y1, x2 - x1);
}

function mult(arr, scale) {
  return arr.map((value) => value * scale);
}

function rotatedTriangle(x, y, size, heading) {
  let shift = (PI * 2) / 3;
  triangle(x + (cos(heading) * size), y + (sin(heading) * size), x + (cos(heading + shift) * size), y + (sin(heading + shift) * size), x + (cos(heading + (shift * 2)) * size), y + (sin(heading + (shift * 2)) * size));
}

function drawPoly(x, y, poly) {
  beginShape();
  for (let i = 0; i < poly.length; i++) {
    let rot = (PI * 2 * i) / poly.length;
    let dist = poly[i];
    vertex(x + (cos(rot) * dist), y + (sin(rot) * dist));
  }
  endShape(CLOSE);
}

class Ant {
  constructor(origin, speed, tracking) {
    this.origin = origin;
    this.speed = speed;
    this.tracking = tracking;

    this.full = false;
    this.returned = false;
    this.x = origin.x;
    this.y = origin.y;
    this.heading = random(0, PI * 2);
    this.life = 1.0;
  }

  tick() {
    this.x += cos(this.heading) * this.speed;
    this.y += sin(this.heading) * this.speed;
    this.heading += random(PI / -30, PI / 30);

    if (this.x < 0 || this.x >= width || this.y < 0 || this.y >= height) {
      let home = angleTo(this.x, this.y, this.origin.x, this.origin.y);
      this.heading = random(home - (PI / 4), home + (PI / 4));
      this.x += cos(this.heading) * this.speed * 2;
      this.y += sin(this.heading) * this.speed * 2;
    }

    if (this.full) {
      window.pheromap.add(this.x, this.y);
      window.ghostmap.add(this.x, this.y);
      let home = angleTo(this.x, this.y, this.origin.x, this.origin.y);
      this.heading = weightedAvg(this.heading % (PI * 2), home, 0.05);

      if (distance(this.x, this.y, this.origin.x, this.origin.y) < window.colony.size / 2) {
        this.returned = true;
      }
    } else {
      let [pheroheading, pherostrength] = window.pheromap.getPheroheading(this.x, this.y);
      this.heading = pheroheading ? weightedAvg(this.heading % (PI * 2), pheroheading, this.tracking * pherostrength) : this.heading;

      let nearest = window.colony.nearestFood(this.x, this.y);
      if (nearest) {
        this.heading = weightedAvg(this.heading % (PI * 2), angleTo(this.x, this.y, nearest.x, nearest.y), 1.0 - (distance(this.x, this.y, nearest.x, nearest.y) / 20.0));
      }

      this.life = Math.pow(this.life, 1.01) * 0.999;
    }
  }

  draw() {
    let color = window.theme.accent;

    noStroke();
    fill(color[0], color[1], color[2]);
    //circle(this.x, this.y, (this.life * 10) + 2);
    rotatedTriangle(this.x, this.y, (this.life * 15) + 4, this.heading);
  }
}

class Colony {
  constructor(x, y, size, rate, numSources) {
    this.size = size;
    this.rate = rate;
    this.x = x;
    this.y = y;

    this.ready = this.size;
    this.ants = [];

    this.color = window.theme.accent;
    this.border = mult(this.color, 0.8);

    this.foodSources = [];
    for (let i = 0; i < numSources; i++) {
      let posX = random(width);
      let posY = random(height);
      this.foodSources.push(new Food(posX, posY, int(random(10, 25))));
    }

    this.shape = [];
    let subdiv = int(random(8, 15));
    let dist = this.size / 2;
    for (let i = 0; i < subdiv; i++) {
      this.shape.push(random(dist, dist + (dist / 3)));
    }
  }

  tick() {
    if (this.ready > 0 && (random(1.0) > 1.0 - this.rate)) {
      this.ants.push(new Ant({ x: this.x, y: this.y }, 4, 0.1));
      this.ready--;
    }

    this.ants.forEach((ant) => ant.tick());

    this.ants.forEach((ant) => {
      this.foodSources.forEach((food) => {
        if (!ant.full && food.touching(ant.x, ant.y)) {
          food.consume();
          ant.full = true;
          ant.heading += PI;
          ant.life = min(1.0, ant.life * 4);
        }
      });
    });

    this.ants = this.ants.filter((ant) => !ant.returned && ant.life > 0.1);
    this.ready = this.size - this.ants.length;

    this.foodSources = this.foodSources.filter((food) => food.left != 0);
  }

  draw() {
    this.ants.forEach((ant) => ant.draw());
    this.foodSources.forEach((food) => food.draw());

    stroke(this.border[0], this.border[1], this.border[2]);
    strokeWeight(5);
    fill(this.color[0], this.color[1], this.color[2]);
    drawPoly(this.x, this.y, this.shape);
  }

  nearestFood(x, y) {
    let minDist = Infinity;
    let minFood = false;

    this.foodSources.forEach((food) => {
      let dist = distance(x, y, food.x, food.y);
      if (dist < 20 && dist < minDist) {
        minDist = dist;
        minFood = food;
      }
    });

    return minFood;
  }
}

class Food {
  constructor(x, y, size) {
    this.x = x;
    this.y = y;
    this.size = size;
    this.left = size;

    this.color = window.theme.food;
    this.border = mult(this.color, 0.8);
  }

  touching(x, y) {
    return distance(this.x, this.y, x, y) < this.left * 1.5;
  }

  consume() {
    if (this.left > 0) this.left--;
    return this.left != 0;
  }

  draw() {
    stroke(this.border[0], this.border[1], this.border[2]);
    strokeWeight(4);
    fill(this.color[0], this.color[1], this.color[2]);
    circle(this.x, this.y, this.left * 3);
  }
}

class Pheromap {
  constructor(width, height, color) {
    this.width = width;
    this.height = height;
    this.color = color;

    this.opacity = 1.0;
    this.map = [];
    for (let y = 0; y < this.height; y++) {
      this.map.push(Array(this.width).fill(0.0));
    }
  }

  query(x, y) {
    let ratioX = this.width / width;
    let ratioY = this.height / height;
    return this.map[clamp(Math.floor(y * ratioY), 0, this.height - 1)]
    [clamp(Math.floor(x * ratioX), 0, this.width - 1)];
  }

  add(x, y) {
    if (distance(x, y, window.colony.x, window.colony.y) < window.colony.size / 2) return;
    let old = this.query(x, y);
    let ratioX = this.width / width;
    let ratioY = this.height / height;
    this.map[clamp(Math.floor(y * ratioY), 0, this.height - 1)]
    [clamp(Math.floor(x * ratioX), 0, this.width - 1)] = min(1.0, old + 0.2);
  }

  draw() {
    let w = width / this.width;
    let h = height / this.height;

    for (let y = 0; y < this.height; y++) {
      for (let x = 0; x < this.width; x++) {
        let strength = this.map[y][x];

        noStroke();
        fill(this.color[0], this.color[1], this.color[2], strength * 200 * this.opacity);
        ellipse((x * w) + (w / 2), (y * h) + (h / 2), w * strength, h * strength);
      }
    }
  }

  onMap(coord) {
    return coord[0] >= 0 && coord[0] < this.width && coord[1] >= 0 && coord[1] < this.height;
  }

  getNeighbors(x, y) {
    let ratioX = this.width / width;
    let ratioY = this.height / height;
    let scaledX = x * ratioX;
    let scaledY = y * ratioY;

    let offsets = [
      [1, 0],
      [1, 1],
      [0, 1],
      [-1, 1],
      [-1, 0],
      [-1, -1],
      [0, -1],
      [1, -1]
    ];

    let coords = offsets.map((off) => [Math.floor(scaledX) + off[0], Math.floor(scaledY) + off[1]]);

    return coords.map((coord) => this.onMap(coord) ? this.map[coord[1]][coord[0]] : 0.0);
  }

  getPheroheading(x, y, show) {
    let neighbors = this.getNeighbors(x, y);
    let sum = neighbors.reduce((acc, a) => acc + a, 0);
    if (show) console.log(neighbors);

    if (sum > 0) {
      let avg = 0.0;
      for (let i = 0; i < neighbors.length; i++) {
        let part = neighbors[i] / sum;
        avg += (part * ((PI * 2) / neighbors.length) * i);
      }

      return [avg, sum / 8.0];
    } else {
      return [false, false];
    }
  }

  fade() {
    this.map = this.map.map((row) => row.map((strength) => strength < 0.1 ? 0.0 : Math.pow(strength, 1.005) * .999));
  }
}

function setup() {
  createCanvas(760, 1360);
  window.theme = window.themes[Math.floor(random(window.themes.length))];

  let posX = random(width / 3, (width / 3) * 2);
  let posY = random(height / 3, (height / 3) * 2);
  window.colony = new Colony(posX, posY, 100, 0.1, 15);

  window.pheromap = new Pheromap(25, 45, window.theme.accent);
  window.ghostmap = new Pheromap(25, 45, window.theme.ghost);
}

function draw() {
  background(window.theme.background[0], window.theme.background[1], window.theme.background[2]);

  window.ghostmap.opacity = 0.7 + ((sin(millis() / 1000) * 0.2));
  window.ghostmap.draw();
  window.pheromap.draw();
  window.colony.draw();
  window.colony.tick();
  window.pheromap.fade();
}