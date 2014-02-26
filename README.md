PHP-WebSocketFrame Extension
============================

This extension provides fast websocket frame decoder / encoder.

https://tools.ietf.org/html/rfc6455

## Status

0.1.0-alpha

## Motivation

There are several websocket pure php implementation but those are really slow
As websocket frame requires xor operation for each bytes.
this extension aims to provide fast websocket frame decoder / encoder.

you can create really `fast` websocket php server with php-uv or PECL event with this extension.
Have Fun!

## API

### Parsing Websocket Frame

````
WebSocketFrame WebSocketFrame::parseFromString($frame);
````

### create a Websocket Frame

````
$frame = new WebSocketFrame();
$frame->setPayload($message);
$frame->setOpcode(WebSocketFrame::OP_TEXT);
echo $frame->serializeToString();
````
