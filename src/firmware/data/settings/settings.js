function showLoadingMessage() {

    $("#alert")
        .removeClass("alert-info alert-danger")
        .addClass("alert-info")
        .text("Loading...");
}

$(function() {
    showLoadingMessage();
    $("form *").prop("disabled", true);
});

$("#save-button").click(showLoadingMessage);

$.ajax("get").done(function(settings) {

    // Display the settings on the page
    $("#tz > option[value=" + settings.timezone + "]").prop("selected", true);
    $("#brightness").val(settings.brightness);
    $("#hour-color").val(settings.hour_color);
    $("#minute-color").val(settings.minute_color);
    $("#second-color").val(settings.second_color);

    $("form *").prop("disabled", false);

    // Display the relevant alert if necessary
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