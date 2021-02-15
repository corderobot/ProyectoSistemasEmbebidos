import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:http/http.dart' as http;

import 'dataPage.dart';

final String _apiSendCommand =
    'http://www.mechanode.com.gt/PDAE/AppSendCommand.php';

final String _apiSendWord = 'http://www.mechanode.com.gt/PDAE/AppSendWord.php';

final String _apiGetCommand =
    'http://www.mechanode.com.gt/PDAE/getAppCommand.php';

enum Colores { r, g, v, w, o }

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  SystemChrome.setPreferredOrientations([DeviceOrientation.portraitUp])
      .then((_) {
    runApp(new MyApp());
  });
}

class MyApp extends StatefulWidget {
  @override
  State<StatefulWidget> createState() => _HomeState();
}

class _HomeState extends State<MyApp> {
  double _currentSliderValue = 0;
  String valLDR = "0";
  Colores color = Colores.v;

  Future<void> _handleSendLed(String comando) async {
    await http.post(_apiSendCommand, body: {"command": comando});
  }

  Future<String> _handleGetLDR() async {
    var valor = await http.get(_apiGetCommand);
    return valor.body;
  }

  Future<void> _handleSendWord(String comando) async {
    final response = await http.post(_apiSendWord, body: {
      "L1": comando.length < 1 ? '' : comando[0],
      "L2": comando.length < 2 ? '' : comando[1],
      "L3": comando.length < 3 ? '' : comando[2],
      "L4": comando.length < 4 ? '' : comando[3],
      "L5": comando.length < 5 ? '' : comando[4],
      "L6": comando.length < 6 ? '' : comando[5],
      "L7": comando.length < 7 ? '' : comando[6],
      "L8": comando.length < 8 ? '' : comando[7],
      "L9": comando.length < 9 ? '' : comando[8],
      "L10": comando.length < 10 ? '' : comando[9],
      "L11": comando.length < 11 ? '' : comando[10],
      "L12": comando.length < 12 ? '' : comando[11],
      "L13": comando.length < 13 ? '' : comando[12],
      "L14": comando.length < 14 ? '' : comando[13],
      "L15": comando.length < 15 ? '' : comando[14],
      "L16": comando.length < 16 ? '' : comando[15],
    });
    print(response.body.toString());
    print(response.request.toString());
  }

  void _handleLedValue(double i) {
    setState(() {
      _currentSliderValue = i;
    });
    switch (i.round()) {
      case 0:
        _handleSendLed("a");
        break;
      case 20:
        _handleSendLed("b");
        break;
      case 40:
        _handleSendLed("c");
        break;
      case 60:
        _handleSendLed("d");
        break;
      case 80:
        _handleSendLed("e");
        break;
      case 100:
        _handleSendLed("f");
        break;
    }
  }

  void _handleLDR() {
    setState(() {
      _handleGetLDR().then((value) => valLDR = value);
    });
  }

  void _handleColor(Colores value) {
    setState(() {
      color = value;
      _handleSendLed(
          value.toString().substring(value.toString().indexOf('.') + 1));
    });
  }

  void _handlePalabra(String value) {
    setState(() {
      _handleSendWord(value.toString());
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      home: Container(
        decoration: BoxDecoration(
          color: Colors.white,
        ),
        child: Scaffold(
            backgroundColor: Colors.transparent,
            appBar: AppBar(
              title: Text("Rodrigo Cordero"),
              backgroundColor:
                  Color(0xFFDFA88C), //Full Opacity add 0xFF at beginning
            ),
            body: DataPage(
                _handleLDR,
                _handleLedValue,
                valLDR,
                _currentSliderValue,
                _handleColor,
                color,
                _handlePalabra)),
      ),
    );
  }
}
