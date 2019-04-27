var filters = {};
var sliders = {};
var weights = {};
var collegeData = {};
var collegeScores = {};

var canvas_width = 550;
var canvas_height = 400;
var padding = 30;  // for chart edges

  // Create scale functions
var xScale = d3.scale.linear()
                .domain([0, 1])
                .range([padding, canvas_width - padding * 2]);

var yScale = d3.scale.linear()
                .domain([0, 2.5])
                .range([canvas_height - padding, padding]);

setupPage();

function setupPage() {
  loadCollegeData();
}

function loadCollegeData() {
  d3.csv("data/college_data.csv", function(data) {
    for (var i = 0; i < data.length; i++) {
      var id = data[i].super_opeid;
      collegeData[id] = {...data[i]}
    }
    setupFilters();
    setupSliders();
  });
}

function setupFilters() {
  d3.json("data/filter-options.json", function(json) {
    for (var i in json) {
      var filterObject = json[i];
      createFilter(filterObject);
    }
  });
}

function setupSliders() {
  d3.json("data/slider-options.json", function(json) {
    for (var i in json) {
      var sliderObject = json[i];
      createFilterButton(sliderObject);
      createSlider(sliderObject);
      createWeightSlider(sliderObject);
    }
    calculateScores();
    displayResults();
    drawGraph();
    colorButtons();
  });
}

function createFilter(filterObject) {
  var category = d3.select("#" + filterObject.type);
  var filter = category.append('div')
    .attr('id', filterObject.id)
    .attr('class', 'filter');
  filter.append('h4').text(filterObject.name);
  filters[filterObject.id] = new Set([]);

  for (var i in filterObject.options) {
    var optionObject = filterObject.options[i];
    filter.append('div')
      .attr('id', optionObject.id)
      .attr('class', 'filter-option active')
      .text(optionObject.name)
      .on("click", flipFilterOptionHandler(filterObject, optionObject));

    filters[filterObject.id].add(optionObject.value);
  }
}

function createFilterButton(sliderObject) {
  var filterButton = d3.select('#filter-buttons')
    .append('div')
    .attr('id', sliderObject.id + '-button')
    .attr('class', 'filter-button')
    .text(sliderObject.name)
    .on('click', clickFilterButtonHandler(sliderObject));
}

function clickFilterButtonHandler(sliderObject) {
  return function() {
    clickFilterButton(sliderObject);
  }
}

function clickFilterButton(sliderObject) {
  d3.selectAll('.filter-button')
    .classed('active', false);
  d3.select('#' + sliderObject.id + '-button')
    .classed("active", true);

  d3.selectAll('.filter-tool').style('display', 'none');
  var filterTool = d3.select("#filter-space")
    .select("#" + sliderObject.id)
    .style('display', 'inline');
}

function createSlider(sliderObject) {
  var slider = d3.sliderHorizontal()
    .domain([sliderObject.min,sliderObject.max])
    .default((sliderObject.max+sliderObject.min)/2)
    .width(150)
    .ticks(2)
    .on('onchange', val => {
      sliders[sliderObject.id] = (slider.value()-sliderObject.min)/(sliderObject.max-sliderObject.min);
      calculateScores();
      displayResults();
      drawGraph()
    })

  var filterTool = d3.select('#filter-space')
    .append('div')
    .style('display', 'none')
    .attr('class', 'filter-tool')
    .attr('id', sliderObject.id);

  filterTool.append('h3').text('Target Value:');

  var filterValue = filterTool
    .append('div')
    .attr('class', 'filter-slider')
    .append('svg')
    .attr('width', 150)
    .attr('height', 50)
    .append('g')
    .call(slider);

  sliders[sliderObject.id] = (slider.value()-sliderObject.min)/(sliderObject.max-sliderObject.min);
}

function createWeightSlider(sliderObject) {
  var category = d3.select("#" + sliderObject.type);

  var slider = d3.sliderHorizontal()
    .max(1)
    .default(0.5)
    .width(150)
    .ticks(2)
    .fill('blue')
    .on('onchange', val => {
      weights[sliderObject.id] = slider.value();
      calculateScores();
      displayResults();
      drawGraph();
      colorButtons();
    })

  var filterTool = d3.select('#' + sliderObject.id);
  filterTool.append('h3').text('Importance:');

  filterTool
    .append('div')
    .attr('class', 'filter-weight')
    .append('svg')
    .attr('width', 150)
    .attr('height', 50)
    .append('g')
    .call(slider);

  weights[sliderObject.id] = slider.value();
}

function flipFilterOptionHandler(filterObject, optionObject) {
  return function() {
    flipFilterOption(filterObject, optionObject);
  }
}

function flipFilterOption(filterObject, optionObject) {
  const active = filters[filterObject.id].has(optionObject.value);
  if (!active) {
    filters[filterObject.id].add(optionObject.value);
  } else {
    filters[filterObject.id].delete(optionObject.value);
  }

  for (var i in collegeData) {
    if (collegeData[i][filterObject.id] === optionObject.value) {
      // ONLY MAKE A CHANGE IF ALL OTHER FILTERS MATCH TOO
      collegeScores[i] = active
        ? 0
        : isCollegeMatch(collegeData[i])
        ? 1
        : 0;
    }
  }
  displayResults();

  var option = d3.select("#" + optionObject.id)
    .classed("active", !active);
}

function isCollegeMatch(collegeObject) {
  for (var i in filters) {
    const filter = filters[i];
    const collegeValue = collegeObject[i];
    if (!filter.has(collegeValue)) {
      return false;
    }
  }
  return true;
}

function displayResults() {
  d3.select("#num-schools").remove();
  d3.select("#college-list").remove();
  var list = d3.select("#results").append('div')
    .attr('id', 'college-list');

  var result = Object.keys(collegeScores).sort(function(a, b) {
    return collegeScores[b] - collegeScores[a];
  }).slice(0,100);

  var counter = 1;
  for (var i in result) {
    id = Number(result[i])
    if (typeof collegeData[id] !== "undefined") {
      createCollegeEntry(collegeData[id], counter);
      counter = counter + 1;
    }
  }

  d3.select("#results").insert('div', '#college-list')
    .attr('id', 'num-schools');
}

function createCollegeEntry(collegeObject, rank) {
  var list = d3.select("#college-list");
  var collegeEntry = list.append('div')
    .attr('class', 'college-entry')
    .attr('id', 'entry-' + collegeObject['super_opeid'])
    .on('click', clickCollegeEntryHandler(collegeObject));

  collegeEntry.append('img')
    .attr('src', 'data/college-pic.png');

  var collegeDescription = collegeEntry.append('div')
    .attr('class', 'college-descr');

  collegeDescription.append('div')
    .attr('class', 'college-title')
    .text(rank.toString() + ". " + collegeObject['name']);

  var subtitle = collegeDescription.append('div')
    .attr('class', 'college-subtitle');
  subtitle.append('span').text('State: ' + collegeObject['state']);
  // subtitle.append('span').text(collegeObject['public'] === '1' ? 'Public' : 'Private');
  subtitle.append('div').text(collegeObject['tier_name']);
}

function clickCollegeEntryHandler(collegeObject) {
  return function() {
    clickCollegeEntry(collegeObject);
  }
}

function clickCollegeEntry(collegeObject) {
  var collegeDetails = d3.select('#school-details');
  collegeDetails.html('');

  collegeDetails.append('h2')
    .text(collegeObject['name']);

  collegeDetails.append('img')
    .attr('src', 'data/pie-chart.png');

  d3.selectAll(".selected-circle")
    .classed("selected-circle", false)
    .attr('r',3)
    .attr('fill','black');

  d3.select('#id-' + collegeObject['super_opeid'])
    .classed("selected-circle", true)
    .attr('fill',d3.rgb('#ffdf00'))
    .attr('r',15);

  d3.selectAll('.selected-entry')
    .classed('selected-entry', false);

  d3.select('#entry-' + collegeObject['super_opeid'])
    .classed('selected-entry', true);

  // createRaceGraph(collegeObject);
}

function createRaceGraph(collegeObject) {
  // set the dimensions and margins of the graph
  var width = 300
      height = 300
      margin = 40

  // The radius of the pieplot is half the width or half the height (smallest one). I substract a bit of margin.
  var radius = Math.min(width, height) / 2 - margin

  // append the svg object to the div called 'my_dataviz'
  var svg = d3.select("#school-details")
    .append("svg")
      .attr("width", width)
      .attr("height", height)
    .append("g")
      .attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");

  var aShare = parseFloat(collegeObject['asian_or_pacific_share_fall_2000']);
  var bShare = parseFloat(collegeObject['black_share_fall_2000']);
  var hShare = parseFloat(collegeObject['hisp_share_fall_2000']);
  otherStat = 1 - (aShare + bShare + hShare);

  // Create dummy data
  var data = {
    'Asian/Pacific': aShare,
    'Black': bShare,
    'Hispanic': hShare,
    'Other': otherStat
  }

  console.log(data);

  // set the color scale
  var color = d3.scaleOrdinal()
    .domain(data)
    .range(["#BCECE0", "#36EEE0", "#F652A0", "#4C5270"])

  // Compute the position of each group on the pie:
  var pie = d3.pie()
    .value(function(d) {return d.value; })
  var data_ready = pie(d3.entries(data))

  // Build the pie chart: Basically, each part of the pie is a path that we build using the arc function.
  svg
    .selectAll('whatever')
    .data(data_ready)
    .enter()
    .append('path')
    .attr('d', d3.arc()
      .innerRadius(0)
      .outerRadius(radius)
    )
    .attr('fill', function(d){ return(color(d.data.key)) })
    .attr("stroke", "black")
    .style("stroke-width", "2px")
    .style("opacity", 0.7)
}

function createPie(data, palette) {
  var svg = d3.select("#school-details")
  .append("svg")
  .append("g")

  svg.append("g")
    .attr("class", "slices");
  svg.append("g")
    .attr("class", "labels");
  svg.append("g")
    .attr("class", "lines");

  var width = 960,
      height = 450,
    radius = Math.min(width, height) / 2;

  var pie = d3.layout.pie()
    .sort(null)
    .value(function(d) {
      return d.value;
    });

  var arc = d3.svg.arc()
    .outerRadius(radius * 0.8)
    .innerRadius(radius * 0.4);

  var outerArc = d3.svg.arc()
    .innerRadius(radius * 0.9)
    .outerRadius(radius * 0.9);

  svg.attr("transform", "translate(" + width / 2 + "," + height / 2 + ")");

  var key = function(d){ return d.data.label; };

  var labels = [];
  for (var i in data) {
    labels.push(data[i]['label']);
  }

  var color = d3.scale.ordinal()
    .domain(labels)
    .range(palette);

  change(data);

  function change(data) {

    /* ------- PIE SLICES -------*/
    var slice = svg.select(".slices").selectAll("path.slice")
      .data(pie(data), key);

    slice.enter()
      .insert("path")
      .style("fill", function(d) { return color(d.data.label); })
      .attr("class", "slice");

    slice
      .transition().duration(1000)
      .attrTween("d", function(d) {
        this._current = this._current || d;
        var interpolate = d3.interpolate(this._current, d);
        this._current = interpolate(0);
        return function(t) {
          return arc(interpolate(t));
        };
      })

    slice.exit()
      .remove();

    /* ------- TEXT LABELS -------*/

    var text = svg.select(".labels").selectAll("text")
      .data(pie(data), key);

    text.enter()
      .append("text")
      .attr("dy", ".35em")
      .text(function(d) {
        return d.data.label;
      });

    function midAngle(d){
      return d.startAngle + (d.endAngle - d.startAngle)/2;
    }

    text.transition().duration(1000)
      .attrTween("transform", function(d) {
        this._current = this._current || d;
        var interpolate = d3.interpolate(this._current, d);
        this._current = interpolate(0);
        return function(t) {
          var d2 = interpolate(t);
          var pos = outerArc.centroid(d2);
          pos[0] = radius * (midAngle(d2) < Math.PI ? 1 : -1);
          return "translate("+ pos +")";
        };
      })
      .styleTween("text-anchor", function(d){
        this._current = this._current || d;
        var interpolate = d3.interpolate(this._current, d);
        this._current = interpolate(0);
        return function(t) {
          var d2 = interpolate(t);
          return midAngle(d2) < Math.PI ? "start":"end";
        };
      });

    text.exit()
      .remove();

    /* ------- SLICE TO TEXT POLYLINES -------*/

    var polyline = svg.select(".lines").selectAll("polyline")
      .data(pie(data), key);

    polyline.enter()
      .append("polyline");

    polyline.transition().duration(1000)
      .attrTween("points", function(d){
        this._current = this._current || d;
        var interpolate = d3.interpolate(this._current, d);
        this._current = interpolate(0);
        return function(t) {
          var d2 = interpolate(t);
          var pos = outerArc.centroid(d2);
          pos[0] = radius * 0.95 * (midAngle(d2) < Math.PI ? 1 : -1);
          return [arc.centroid(d2), outerArc.centroid(d2), pos];
        };
      });

    polyline.exit()
      .remove();
  };
}

function correlationScore(collegeObject) {
  var score = 0;
  var sum_w = 0;
  for (var i in sliders) {
    var dif = 1-Math.abs(sliders[i]-collegeObject[i + '_norm']);
    score += weights[i]*dif;
    sum_w += weights[i];
  }
  return score/sum_w;
}

function calculateScores() {
  for (var i in collegeData) {
    collegeScores[i] = correlationScore(collegeData[i]);
  }
}

function colorButtons() {
  for (var f in weights) {
    var color = 255 - (weights[f] * 150 + 60);
    var filterButton = d3.select('#' + f + '-button')
      .style('background-color', 'rgba(0,' + color.toString() + ',255)');
  }
}

function drawGraph() {
  d3.select("#scatterplot").remove();

  var data = [];
  for (var i in collegeData) {
    data.push([collegeScores[i],Number(collegeData[i]['k_mean'])/Number(collegeData[i]['par_mean']),i]);
  }

  var xAxis = d3.axisBottom().scale(xScale).ticks(5);
  var yAxis = d3.axisLeft().scale(yScale).tickFormat(d3.format("")).ticks(5);

  var div = d3.select("#graph").append("div")
    .attr("class", "tooltip")
    .style("opacity", 0);

  var svg = d3.select("#graph")
              .append("svg")
              .attr("id","scatterplot")
              .attr("width", canvas_width)
              .attr("height", canvas_height + padding);

  svg.selectAll("circle")
     .data(data).enter()
     .append("circle")
     .attr("id", function(d) {
        return "id-" + d[2];
     })
     .attr("cx", function(d) {
        return xScale(d[0]) + padding;
     })
     .attr("cy", function(d) {
        return yScale(d[1]);
     })
     .attr("r", 3)
     .attr("opacity",0.5)
     .on("mouseover", function(d) {
            div.transition()
                .duration(200)
                .style("opacity", .9);
            div.html(collegeData[d[2]]['name'])
                .style("left", (d3.event.pageX) + "px")
                .style("top", (d3.event.pageY - 28) + "px");
            })
        .on("mouseout", function(d) {
            div.transition()
               .duration(500)
               .style("opacity", 0);
        });

   svg.append("g")
      .attr("transform", "translate(" + padding + "," + (canvas_height - padding) +")")
      .call(xAxis);

    svg.append("text")
        .attr("transform",
              "translate(" + (canvas_width/2 + padding) + " ," +
                             (canvas_height - padding/2 + 20) + ")")
        .style("text-anchor", "middle")
        .text("Match Score");

  svg.append("g")
      .attr("transform", "translate(" + (2*padding) +",0)")
      .call(yAxis);

  svg.append("text")
      .attr("transform", "rotate(-90)")
      .attr("y", 0)
      .attr("x",0 - (canvas_height / 2))
      .attr("dy", "1em")
      .style("text-anchor", "middle")
      .text("Ratio of Average Child to Parent Income");
}
