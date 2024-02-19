console.log("injected javascript hehe");

  // Establish WebSocket connection
  const socket = new WebSocket('ws://localhost:8080');

  // Log errors
  socket.onerror = function(error) {
    console.error('WebSocket error:', error);
  };

  // Log messages from the server
  socket.onmessage = function(event) {
    console.log('Message from server:', event.data);
    
    // You can handle the message here, for example, if the message indicates a page reload:
    if (event.data === 'reload') {
      location.reload();
    }
  };

  // Optionally, you can send messages to the server
  // socket.send('Hello, server!');