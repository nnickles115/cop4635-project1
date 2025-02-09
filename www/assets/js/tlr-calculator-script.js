/**
 * file: tlr-calculator-script.js
 * brief: Helps calculating and managing time based on user inputs.
 *
 * author: Noah Nickles
 * updated: 1/30/2025
 * personal email: nnickles98@gmail.com
 * student email: npn4@students.uwf.edu
 */

'use strict';

// Globals //

let pieChart;
let isInitialLoad = true;

// Constants //

const totalHours = 168;
const colorMapping = {
    'credit-hours':  '--nautilus_blue',
    'study-hours':   '--luna_blue',
    'work-hours':    '--cannon_green',
    'commute-hours': '--spring_green',
    'eating-hours':  '--armadillo_grey',
    'sleeping-hours':'--uwf_blue',
    'hygiene-hours': '--uwf_green',
    'org-hours':     '--earth',
    'rec-hours':     '--camelia',
    'social-hours':  '--marigold',
    'screen-hours':  '--azalea',
    'other-hours':   '--ocean'
};
const inputCategories = [
    'credit-hours',
    'study-hours',
    'work-hours',
    'commute-hours',
    'eating-hours',
    'sleeping-hours',
    'hygiene-hours',
    'org-hours',
    'rec-hours',
    'chore-hours',
    'social-hours',
    'screen-hours',
    'other-hours'
];

// Getters //

/**
 * Helper function to get the CSS variable color.
 *
 * @param {string} variableName - The name of the CSS variable.
 * @returns {string} - The CSS variable value.
 */
function getColorVariable(variableName) {
    return getComputedStyle(document.documentElement).getPropertyValue(variableName).trim();
}

/**
 * Retrieves data, labels, and background colors for the chart from input categories.
 *
 * @returns {object} - An object containing the requested info.
 */
function getDataForChart() {
    let totalHoursUsed = 0;
    const dataValues = [];
    const dataLabels = [];
    const backgroundColors = [];

    for(const category of inputCategories) {
        const inputElement = document.getElementById(category);
        const hours = parseInt(inputElement.value, 10) || 0;
        totalHoursUsed += hours;

        dataValues.push(hours);
        dataLabels.push(inputElement.labels[0].textContent);
        backgroundColors.push(getColorVariable(colorMapping[category]));
    }

    return { totalHoursUsed, dataValues, dataLabels, backgroundColors };
}

// Update UI //

/**
 * Updates the 'hours left' display on the page.
 *
 * @param {number} totalHoursUsed - The total hours used so far.
 */
function updateHoursLeftDisplay(totalHoursUsed) {
    const hoursLeftSpan = document.getElementById('hours-left');
    let hoursLeft = totalHours - totalHoursUsed;

    if(hoursLeft < 0) hoursLeft = 0;
    hoursLeftSpan.textContent = hoursLeft;
    return hoursLeft; // Return hoursLeft for use in other functions if needed
}

/**
 * Clears the percentage spans and resets their background color.
 */
function clearPercentageSpans() {
    inputCategories.forEach(category => {
        const percentageSpan = document.getElementById(`${category}-percentage`);
        percentageSpan.textContent = '';
        percentageSpan.style.backgroundColor = 'transparent';
    });
}

/**
 * Updates the percentage spans next to each input field.
 *
 * @param {Array<number>} dataValues - Array of hours for each category.
 * @param {number} currentTotalHours - The total hours being considered (used + remaining).
 * @param {Array<string>} backgroundColors - Array of background colors for each category.
 */
function updatePercentageSpans(dataValues, currentTotalHours, backgroundColors) {
    for(let i = 0; i < inputCategories.length; i++) {
        const category = inputCategories[i];
        const percentageSpan = document.getElementById(`${category}-percentage`);
        const percentage = ((dataValues[i] / currentTotalHours) * 100).toFixed(1);
        percentageSpan.textContent = isNaN(percentage) ? '' : `${percentage}%`;
        percentageSpan.style.backgroundColor = backgroundColors[i];
    }
}

// Pie Chart //

/**
 * Initializes or updates the pie chart.
 *
 * @param {Array<string>} dataLabels - Labels for the chart segments.
 * @param {Array<number>} dataValues - Data values for the chart segments.
 * @param {Array<string>} backgroundColors - Background colors for the chart segments.
 * @param {number} currentTotalHours - The total hours used in calculations.
 */
function updatePieChart(dataLabels, dataValues, backgroundColors, currentTotalHours) {
    // Helper consts for chart settings (avoids deep nesting) //
    const chartTooltip = {
        animation: {
            duration: 300,
            easing: 'easeOutQuad'
        },
        callbacks: {
            label: getToolTipLabel(currentTotalHours),
            labelColor: getToolTipLabelColor()
        }
    };
    
    const chartPlugins = {
        tooltip: chartTooltip,
        legend: { display: false }
    };

    const chartData = {
        labels: dataLabels,
        datasets: [{
            data: dataValues,
            backgroundColor: backgroundColors,
            hoverOffset: 4
        }]
    };
    
    const chartOptions = {
        responsive: true,
        maintainAspectRatio: false,
        animation: {
            animateRotate: true,
            animateScale: true,
            duration: 1000,
            easing: 'easeOutQuart'
        },
        plugins: chartPlugins
    };

    if(!pieChart) { // Initialize new chart
        const ctx = document.getElementById('pie-chart').getContext('2d');
        pieChart = new Chart(ctx, {
            type: 'pie',
            data: chartData,
            options: chartOptions
        });
        isInitialLoad = false;
    } 
    else { // Update existing chart
        pieChart.data.labels = dataLabels;
        pieChart.data.datasets[0].data = dataValues;
        pieChart.data.datasets[0].backgroundColor = backgroundColors;
        pieChart.options.animation.duration = 750; // Ensure animation on updates
        pieChart.options.animation.easing = 'easeOutQuart';
        pieChart.update();
    }
}

/**
 * Helper function to get the tooltip label color based on the context.
 * @returns {function} - A function that returns the tooltip label color based on the context.
 */
function getToolTipLabelColor() {
    return(context) => {
        return {
            borderColor: 'rgb(0, 0, 0)',
            backgroundColor: context.dataset.backgroundColor[context.dataIndex],
            borderWidth: 2,
        };
    };
}

/**
 * Helper function to get the tooltip label based on the context.
 * @param {number} currentTotalHours - The total hours used in calculations. 
 * @returns {function} - A function that returns the tooltip label based on the context.
 */
function getToolTipLabel(currentTotalHours) {
    return(context) => {
        let label = context.label || '';
        if(label) label += ': ';
        if(context.parsed !== null) {
            const percentage = ((context.parsed / currentTotalHours) * 100).toFixed(1);
            label += context.parsed + ' (' + percentage + '%)';
        }
        return label;
    };
}

// Validation //

/**
 * Validates the input value to prevent negative numbers and exceeding total hours.
 *
 * @param {HTMLElement} inputElement - The input element being validated.
 */
function validateInputValue(inputElement) {
    let currentValue = parseInt(inputElement.value, 10) || 0;

    // Prevent negative numbers
    if(currentValue < 0) {
        currentValue = 0;
        inputElement.value = '0';
    }

    // Prevent exceeding total hours
    const previousValue = parseInt(inputElement.dataset.previousValue || inputElement.defaultValue, 10) || 0;
    let totalHoursUsedBeforeChange = 0;

    // Calculate total hours used before the current input change
    for(const cat of inputCategories) {
        if(cat !== inputElement.id) {
            totalHoursUsedBeforeChange += parseInt(document.getElementById(cat).value, 10) || 0;
        }
    }

    // Calculate predicted total hours used and hours left
    const predictedTotalHoursUsed = totalHoursUsedBeforeChange + currentValue;
    const predictedHoursLeft = totalHours - predictedTotalHoursUsed;

    // Prevent exceeding total hours
    if(predictedHoursLeft < 0) {
        inputElement.value = previousValue;
        return false;
    }

    // Update previous value for future validation
    inputElement.dataset.previousValue = inputElement.value;
    return true;
}

/**
 * Handles empty input on blur (HTML loses focus due to input element) by setting it to '0'.
 *
 * @param {HTMLElement} inputElement - The input element that lost focus.
 */
function handleEmptyInputBlur(inputElement) {
    if(inputElement.value === '') {
        inputElement.value = '0';
        inputElement.dataset.previousValue = '0';
        updateChart();
    }
}

// Synchronization //

/**
 * Syncs the slider and credit hours input values.
 *
 * @param {string} inputValue - The value to sync to both slider and credit hours input.
 */
function syncSliderAndCreditHours(inputValue) {
    const slider = document.getElementById('slider');
    const sliderCreditsSpan = document.getElementById('slider-credits');
    const creditHoursInput = document.getElementById('credit-hours');

    slider.value = inputValue;
    sliderCreditsSpan.textContent = inputValue;
    creditHoursInput.value = inputValue;
    creditHoursInput.dataset.previousValue = inputValue;
}

// Reset //

/**
 * Resets all input values to their default states.
 */
function resetInputValues() {
    // Reset slider and credit hours first
    syncSliderAndCreditHours('3');

    document.getElementById('study-hours').value    =  '3';
    document.getElementById('work-hours').value     =  '0';
    document.getElementById('commute-hours').value  =  '0';
    document.getElementById('eating-hours').value   =  '0';
    document.getElementById('sleeping-hours').value = '56';
    document.getElementById('hygiene-hours').value  =  '0';
    document.getElementById('org-hours').value      =  '0';
    document.getElementById('rec-hours').value      =  '0';
    document.getElementById('chore-hours').value    =  '0';
    document.getElementById('social-hours').value   =  '0';
    document.getElementById('screen-hours').value   =  '0';
    document.getElementById('other-hours').value    =  '0';
}

// Main //

/**
 * Main function to update the chart based on input values.
 */
function updateChart() {
    const chartData = getDataForChart();
    const hoursLeft = updateHoursLeftDisplay(chartData.totalHoursUsed);

    /*
     * Clear chart and percentage spans if all hours are used.
     * This is to prevent the chart from updating past 100% when all hours are used.
     */
    if(hoursLeft === totalHours) {
        if(pieChart) {
            pieChart.data.datasets[0].data = [];
            pieChart.data.labels = [];
            pieChart.update();
        }
        clearPercentageSpans();
        return;
    }

    // Update chart data using spread operator to avoid modifying original data (shallow copy)
    let dataValues = [...chartData.dataValues];
    let dataLabels = [...chartData.dataLabels];
    let backgroundColors = [...chartData.backgroundColors];

    // Add hours remaining to the chart if there are any
    if(hoursLeft > 0) {
        dataValues.push(hoursLeft);
        dataLabels.push("Hours Remaining");
        backgroundColors.push(getColorVariable('--background-grey'));
    }

    // Update total hours used for percentage calculations
    let currentTotalHours = chartData.totalHoursUsed;
    if(hoursLeft > 0) currentTotalHours += hoursLeft;

    // Update percentage spans and pie chart with new data
    updatePercentageSpans(dataValues, currentTotalHours, backgroundColors);
    updatePieChart(dataLabels, dataValues, backgroundColors, currentTotalHours);
}

// Event Listeners //

inputCategories.forEach(category => {
    const inputElement = document.getElementById(category);

    inputElement.addEventListener('input', function() {
        if(validateInputValue(this)) updateChart();
    });

    inputElement.addEventListener('blur', function() {
        handleEmptyInputBlur(this);
    });
});


document.getElementById('credit-hours').addEventListener('input', function() {
    if(validateInputValue(this)) {
        syncSliderAndCreditHours(this.value);
        updateChart();
    }
});

document.getElementById('credit-hours').addEventListener('blur', function() {
    handleEmptyInputBlur(this);
});


document.getElementById('slider').addEventListener('input', function() {
    const creditHoursInput = document.getElementById('credit-hours');
    const currentValue = parseInt(this.value, 10) || 0;
    const previousValue = parseInt(creditHoursInput.dataset.previousValue || creditHoursInput.defaultValue, 10) || 0;
    let totalHoursUsedBeforeChange = 0;

    for(const cat of inputCategories) {
        if(cat !== 'credit-hours') { // Sum hours except credit-hours
            totalHoursUsedBeforeChange += parseInt(document.getElementById(cat).value, 10) || 0;
        }
    }
    const predictedTotalHoursUsed = totalHoursUsedBeforeChange + currentValue;
    const predictedHoursLeft = totalHours - predictedTotalHoursUsed;


    if(predictedHoursLeft < 0) {
        this.value = previousValue; // Revert slider value
        syncSliderAndCreditHours(previousValue.toString());
        return;
    }

    syncSliderAndCreditHours(this.value);
    updateChart();
});

document.getElementById('reset-button').addEventListener('click', function() {
    resetInputValues();
    updateChart();
});

// Initializations //

// Initialize slider value from credit hours input on page load
syncSliderAndCreditHours(document.getElementById('credit-hours').value);

// Initial chart setup on page load
updateChart();