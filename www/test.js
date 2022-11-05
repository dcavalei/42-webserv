function f() {
    let request = new Request("/index.html?yo", {method: "POST", body: "yoyoyoyo"});
    fetch(request)
        .then(data => {
            const reader = data.body.getReader();
            let charsReceived = 0;
            console.log(reader);

            reader.read().then(function processText({done, value}) {
                charsReceived += value.length;
                console.log(value);
                let content = new TextDecoder().decode(value);
                console.log(content);
            });
        });
}
