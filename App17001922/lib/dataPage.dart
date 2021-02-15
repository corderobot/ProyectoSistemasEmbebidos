import 'package:App17001922/main.dart';
import "package:flutter/material.dart";

class DataPage extends StatelessWidget {
  final String valLDR;
  final double currentSliderValue;
  final Function updateLDR, updateLed, updateColor, updateWord;
  final Colores colorSel;
  final myController = TextEditingController();

  DataPage(this.updateLDR, this.updateLed, this.valLDR, this.currentSliderValue,
      this.updateColor, this.colorSel, this.updateWord);

  @override
  Widget build(BuildContext context) {
    return Container(
      color: Color(0xFFE0C9C2),
      child: ListView(
        children: [
          Column(
            children: [
              Text(
                "Proyecto Sistemas Embebidos",
                style: TextStyle(fontSize: 20),
              ),
              SizedBox(height: 25),
              Text(
                "LDR = " + valLDR,
                style: TextStyle(fontSize: 20),
              ),
              SizedBox(height: 25),
              GestureDetector(
                onTap: () => updateLDR(),
                child: new Container(
                    margin: const EdgeInsets.all(0.0),
                    child: new Icon(
                      Icons.refresh,
                      color: Colors.black,
                      size: 50.0,
                    )),
              ),
              SizedBox(height: 25),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text("Led:"),
                  Slider(
                    value: currentSliderValue,
                    min: 0,
                    max: 100,
                    divisions: 5,
                    label: currentSliderValue.round().toString() + "%",
                    onChanged: (double value) {
                      updateLed(value);
                    },
                  ),
                ],
              ),
              SizedBox(height: 25),
              ListTile(
                title: const Text('Rojo'),
                leading: Radio(
                  value: Colores.r,
                  groupValue: colorSel,
                  onChanged: (Colores value) {
                    updateColor(value);
                  },
                ),
              ),
              ListTile(
                title: const Text('Verde'),
                leading: Radio(
                  value: Colores.g,
                  groupValue: colorSel,
                  onChanged: (Colores value) {
                    updateColor(value);
                  },
                ),
              ),
              ListTile(
                title: const Text('Azul'),
                leading: Radio(
                  value: Colores.v,
                  groupValue: colorSel,
                  onChanged: (Colores value) {
                    updateColor(value);
                  },
                ),
              ),
              ListTile(
                title: const Text('Blanco'),
                leading: Radio(
                  value: Colores.w,
                  groupValue: colorSel,
                  onChanged: (Colores value) {
                    updateColor(value);
                  },
                ),
              ),
              ListTile(
                title: const Text('Apagar'),
                leading: Radio(
                  value: Colores.o,
                  groupValue: colorSel,
                  onChanged: (Colores value) {
                    updateColor(value);
                  },
                ),
              ),
              SizedBox(height: 25),
              TextField(
                controller: myController,
              ),
              SizedBox(height: 25),
              RaisedButton(
                  child: Text("Enviar Palabra"),
                  onPressed: () => updateWord(myController.text)),
            ],
          ),
        ],
      ),
    );
  }
}
