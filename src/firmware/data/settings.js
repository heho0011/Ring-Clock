// Display loading message when form is submitted
$("#save-button").click(function() {
    $("#alert")
        .removeClass("alert-info alert-danger")
        .addClass("alert-info")
        .text("Loading...");
})

// Get the settings from the server
$.ajax("/getSettings").done(function(settings) {

    // Display the settings on the page
    $("#tz > option[value=" + settings.timezone + "]").prop("selected", true);

    if (location.search.indexOf("status=success") != -1) {

        $("#alert")
            .removeClass("alert-info")
            .addClass("alert-success")
            .text("Settings saved successfully!");

    } else if (location.search.indexOf("status=error") != -1) {

        $("#alert")
            .removeClass("alert-info")
            .addClass("alert-danger")
            .text("An error occurred when saving the settings.");

    } else {

        $("#alert")
            .removeClass("alert-info")
            .text("");
            
    }
});