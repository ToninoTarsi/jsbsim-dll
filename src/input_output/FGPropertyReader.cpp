/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Module:       FGPropertyReader.cpp
 Author:       Bertrand Coconnier
 Date started: 12/30/13
 Purpose:      Read and manage properties from XML data

  ------------- Copyright (C) 2013 Bertrand Coconnier -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU Lesser General Public License can also be found on
 the world wide web at http://www.gnu.org.

FUNCTIONAL DESCRIPTION
--------------------------------------------------------------------------------
This class reads and manages properties defined in XML data

HISTORY
--------------------------------------------------------------------------------
12/30/13   BC    Created

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include "FGPropertyReader.h"
#include "FGPropertyManager.h"
#include "FGXMLElement.h"
#include "FGJSBBase.h"

using namespace std;

namespace JSBSim {

static const char *IdSrc = "$Id: FGPropertyReader.cpp 16671 2014-01-07 12:06:05Z dolan.paul $";
static const char *IdHdr = ID_PROPERTYREADER;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS IMPLEMENTATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

bool FGPropertyReader::ResetToIC(void)
{
  map<FGPropertyNode_ptr, double>::iterator it = interface_prop_initial_value.begin();
  for (;it != interface_prop_initial_value.end(); ++it) {
    FGPropertyNode* node = it->first;
    if (!node->getAttribute(SGPropertyNode::PRESERVE))
      node->setDoubleValue(it->second);
  }

  return true;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void FGPropertyReader::LoadProperties(Element* el, FGPropertyManager* PM,
				      bool override)
{
  // Interface properties are all stored in the interface properties array.
  string interface_property_string = "";

  Element *property_element = el->FindElement("property");
  if (property_element && FGJSBBase::debug_lvl > 0) {
    cout << endl << "    ";
    if (override)
      cout << "Overriding";
    else
      cout << "Declared";
    cout << " properties" << endl << endl;
  }

  while (property_element) {
    FGPropertyNode* node = 0;
    double value=0.0;
    if ( ! property_element->GetAttributeValue("value").empty())
      value = property_element->GetAttributeValueAsNumber("value");

    interface_property_string = property_element->GetDataLine();
    if (PM->HasNode(interface_property_string)) {
      if (override) {
        node = PM->GetNode(interface_property_string);

        if (FGJSBBase::debug_lvl > 0) {
          if (interface_prop_initial_value.find(node) == interface_prop_initial_value.end()) {
            cout << property_element->ReadFrom()
                 << "  The following property will be overridden but it has not been" << endl
                 << "  defined in the current model '" << el->GetName() << "'" << endl;
          }

          cout << "      " << "Overriding value for property " << interface_property_string << endl
               << "       (old value: " << node->getDoubleValue() << "  new value: " << value << ")"
               << endl << endl;
        }

        node->setDoubleValue(value);
      }
      else {
        cerr << property_element->ReadFrom()
             << "      Property " << interface_property_string 
             << " is already defined." << endl;
	property_element = el->FindNextElement("property");
	continue;
      }
    } else {
      interface_properties.push_back(value);
      PM->Tie(interface_property_string, &interface_properties.back());
      if (FGJSBBase::debug_lvl > 0)
        cout << "      " << interface_property_string << " (initial value: " 
             << value << ")" << endl << endl;
      node = PM->GetNode(interface_property_string);
    }
    interface_prop_initial_value[node] = value;
    if (property_element->GetAttributeValue("persistent") == string("true"))
      node->setAttribute(SGPropertyNode::PRESERVE, true);

    property_element = el->FindNextElement("property");
  }
  
  // End of interface property loading logic
}
}
