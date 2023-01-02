var editor = CodeMirror.fromTextArea(document.getElementById("#docs"),{
    lineNumbers: true,
    mode:"python",
    theme: "monokai"
});

$(document).ready(function () {
    function get_current_line_num() {
        const t = $("#docs")[0];
        return editor.getCode().split("\n").length;
    }

    $("#docs").load("/docs_reader");
    editor.setValue($('#docs').val);

    // $("textarea#docs").on("keyup keypress keydown change", function () {
    $('#docs').bind('input propertychange', function() {
        const line_num = get_current_line_num();

        const textarea = $(this);
        const text = textarea.val().split("\n");
        if (text.length < line_num) {
            return;
        }
        let text_line = text[line_num - 1];
        text_line = text_line.replaceAll(" ", "+");

        if (text_line.trim().length == 0) {
            return;
        }

        const url = "/docs_writer?line_num=" + line_num + "&content=" + text_line;
        $.get(url, (data, status) => {
            if (status == "success") {
                editor.setValue(data);
            }
        });
    });
});
